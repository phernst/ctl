#include "xrayspectrummodels.h"
#include <cmath>
#include <QDebug>

namespace CTL {

// ____________________________
// # AbstractXraySpectrumModel
// ----------------------------
void AbstractXraySpectrumModel::setParameter(const QVariant &parameter)
{
    if(parameter.canConvert(QMetaType::Float))
        _energy = parameter.toFloat();
    else
        _energy = parameter.toMap().value("energy").toFloat();
}

QVariant AbstractXraySpectrumModel::parameter() const
{
    return QVariantMap{std::make_pair(QString("energy"), _energy)};
}

// _____________________________
// # XraySpectrumTabulatedModel
// -----------------------------
float XraySpectrumTabulatedModel::valueAt(float position) const
{
    if(!hasTabulatedDataFor(_energy))
        throw std::domain_error("No tabulated data available for parameter value: "
                                + std::to_string(_energy));

    // check if exact lookup is available --> return corresponding data
    if(_lookupTables.contains(_energy))
        return _lookupTables.value(_energy).valueAt(position);

    // data needs to be interpolated for requested value of '_parameter'
    auto parPosition = _lookupTables.upperBound(_energy);
    // --> always gives next tabulated data, since no exact lookup available

    auto tableLower = (parPosition - 1).value(); // tabulated data with voltage < _parameter
    auto tableUpper = (parPosition).value(); // tabulated data with voltage > _parameter

    auto lowerValue = tableLower.valueAt(position);
    auto upperValue = tableUpper.valueAt(position);

    // linear interpolation weighting
    auto weightFactor
        = (parPosition.key() - _energy) / (parPosition.key() - (parPosition - 1).key());

    return lowerValue * weightFactor + upperValue * (1.0f - weightFactor);
}

float XraySpectrumTabulatedModel::binIntegral(float position, float binWidth) const
{
    //    qDebug() << "value at called with: " << _parameter << samplePoint << spacing;

    if(!hasTabulatedDataFor(_energy))
        throw std::domain_error("No tabulated data available for parameter value: "
                                + std::to_string(_energy));

    // check if exact lookup is available --> return corresponding data
    if(_lookupTables.contains(_energy))
        return _lookupTables.value(_energy).binIntegral(position, binWidth);

    // data needs to be interpolated for requested value of '_parameter'
    auto parPosition = _lookupTables.upperBound(_energy);
    // --> always gives next tabulated data, since no exact lookup available

    auto tableLower = (parPosition - 1).value(); // tabulated data with voltage < _parameter
    auto tableUpper = (parPosition).value(); // tabulated data with voltage > _parameter

    //    qDebug() << _parameter << ":" << parPosition.key() << parPosition.value();
    //    qDebug() << "call Integrate: [" << samplePoint-spacing/2.0 << "," <<
    //    samplePoint+spacing/2.0 << "] ";

    auto lowerIntegral = tableLower.binIntegral(position, binWidth);
    auto upperIntegral = tableUpper.binIntegral(position, binWidth);

    //    qDebug() << "lowerIntegral = " << lowerIntegral;
    //    qDebug() << "upperIntegral = " << upperIntegral;

    // linear interpolation weighting
    auto weightFactor
        = (parPosition.key() - _energy) / (parPosition.key() - (parPosition - 1).key());

    return lowerIntegral * weightFactor + upperIntegral * (1.0f - weightFactor);
}

AbstractDataModel *XraySpectrumTabulatedModel::clone() const
{
    return new XraySpectrumTabulatedModel(*this);
}

QVariant XraySpectrumTabulatedModel::toVariant() const
{
    auto variant = AbstractDataModel::toVariant().toMap();
    QVariantList dataList;

    auto i = _lookupTables.constBegin();
    while (i != _lookupTables.constEnd())
    {
        QVariantMap map;
        map.insert("table voltage", i.key());
        map.insert("table data", i.value().toVariant());

        dataList.append(map);
        ++i;
    }

    variant.insert("lookup tables", dataList);

    return variant;
}

void XraySpectrumTabulatedModel::fromVariant(const QVariant& variant)
{
    AbstractDataModel::fromVariant(variant);

    _lookupTables.clear();

    // populate lookup table
    auto lookupTableData = variant.toMap().value("lookup tables").toList();
    foreach(const QVariant& var, lookupTableData)
    {
        // each "var" represents a lookup table for a certain tube voltage
        auto varAsMap = var.toMap();
        auto voltage = varAsMap.value("table voltage").toFloat();
        auto tableData = varAsMap.value("table data");

        TabulatedDataModel table;
        table.fromVariant(tableData);

        addLookupTable(voltage, table);
    }
}

void XraySpectrumTabulatedModel::setLookupTables(const QMap<float, TabulatedDataModel>& tables)
{
    _lookupTables = tables;
}

void XraySpectrumTabulatedModel::addLookupTable(float voltage, const TabulatedDataModel& table)
{
    _lookupTables.insert(voltage, table);
}

bool XraySpectrumTabulatedModel::hasTabulatedDataFor(float voltage) const
{
    if(_lookupTables.isEmpty())
        return false;

    return (voltage >= _lookupTables.firstKey() && voltage <= (_lookupTables.lastKey()));
}


// _____________________________
// # XrayLaserSpectrumModel
// -----------------------------
float XrayLaserSpectrumModel::valueAt(float position) const
{
    if(qFuzzyCompare(position,_energy))
        return 1.0f;
    else
        return 0.0f;
}


float XrayLaserSpectrumModel::binIntegral(float position, float binWidth) const
{
    if((_energy >= position - 0.5f*binWidth) && (_energy <= position + 0.5f*binWidth))
        return 1.0f;
    else
        return 0.0f;
}

AbstractDataModel *XrayLaserSpectrumModel::clone() const
{
    return new XrayLaserSpectrumModel(*this);
}


// _____________________________
// # FixedXraySpectrumModel
// -----------------------------
FixedXraySpectrumModel::FixedXraySpectrumModel(const TabulatedDataModel &table)
{
    addLookupTable(0.0f, table);
}

void FixedXraySpectrumModel::setLookupTable(const TabulatedDataModel& table)
{
    QMap<float, TabulatedDataModel> tmp;
    tmp.insert(0.0f, table);
    setLookupTables(tmp);
}

// _____________________________
// # KramersLawSpectrumModel
// -----------------------------
float KramersLawSpectrumModel::valueAt(float position) const
{
    return (position<_energy) ? (_energy / position - 1.0f) : 0.0f;
}

float KramersLawSpectrumModel::binIntegral(float position, float binWidth) const
{
    static const float LOW_END = 0.1f;

    float bot = position - 0.5f*binWidth;
    float top = position + 0.5f*binWidth;

    if((top < LOW_END) || (bot > _energy))
        return 0.0f;

    if(bot < LOW_END)
        bot = LOW_END;
    if(top > _energy)
        top = _energy;

    return _energy * log(top / bot) - (top - bot);
}

AbstractDataModel *KramersLawSpectrumModel::clone() const
{
    return new KramersLawSpectrumModel(*this);
}

} // namespace CTL
