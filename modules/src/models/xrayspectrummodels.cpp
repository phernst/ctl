#include "xrayspectrummodels.h"
#include <cmath>
#include <QDebug>

namespace CTL {

// ____________________________
// # AbstractXraySpectrumModel
// ----------------------------
void AbstractXraySpectrumModel::setParameter(const float& voltage) { _parameter = voltage; }

const float& AbstractXraySpectrumModel::parameter() const { return _parameter; }

// _____________________________
// # XraySpectrumTabulatedModel
// -----------------------------
QVariant XraySpectrumTabulatedModel::toVariant() const { return QVariant(); }

void XraySpectrumTabulatedModel::fromVariant(const QVariant& variant)
{
    return;
}

void XraySpectrumTabulatedModel::setLookupTables(const QMap<float, TabulatedModelData>& tables)
{
    _lookupTables = tables;
}

void XraySpectrumTabulatedModel::addLookupTable(float voltage, const TabulatedModelData& table)
{
    _lookupTables.insert(voltage, table);
}

float XraySpectrumTabulatedModel::valueFromModel(float samplePoint, float spacing) const
{
    //    qDebug() << "value at called with: " << _parameter << samplePoint << spacing;

    if(!hasTabulatedDataFor(_parameter))
        throw std::domain_error("No tabulated data available for parameter value: "
                                + std::to_string(_parameter));

    const auto leftBinPosition = samplePoint - spacing / 2.0f;
    const auto rightBinPosition = samplePoint + spacing / 2.0f;

    // check if exact lookup is available --> return corresponding data
    if(_lookupTables.contains(_parameter))
        return _lookupTables.value(_parameter).trapezoidIntegral(leftBinPosition, rightBinPosition);

    // data needs to be interpolated for requested value of '_parameter'
    auto parPosition = _lookupTables.upperBound(_parameter);
    // --> always gives next tabulated data, since no exact lookup available

    auto tableLower = (parPosition - 1).value(); // tabulated data with voltage < _parameter
    auto tableUpper = (parPosition).value(); // tabulated data with voltage > _parameter

    //    qDebug() << _parameter << ":" << parPosition.key() << parPosition.value();
    //    qDebug() << "call Integrate: [" << samplePoint-spacing/2.0 << "," <<
    //    samplePoint+spacing/2.0 << "] ";

    auto lowerIntegral = tableLower.trapezoidIntegral(leftBinPosition, rightBinPosition);
    auto upperIntegral = tableUpper.trapezoidIntegral(leftBinPosition, rightBinPosition);

    //    qDebug() << "lowerIntegral = " << lowerIntegral;
    //    qDebug() << "upperIntegral = " << upperIntegral;

    // linear interpolation weighting
    auto weightFactor
        = (parPosition.key() - _parameter) / (parPosition.key() - (parPosition - 1).key());

    return lowerIntegral * weightFactor + upperIntegral * (1.0f - weightFactor);
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
float XrayLaserSpectrumModel::valueFromModel(float samplePoint, float spacing) const
{
    if((_parameter >= samplePoint - 0.5f*spacing) && (_parameter <= samplePoint + 0.5f*spacing))
        return 1.0f;
    else
        return 0.0f;
}

QVariant XrayLaserSpectrumModel::toVariant() const { return QVariant(); }

void XrayLaserSpectrumModel::fromVariant(const QVariant&) { return; }


// _____________________________
// # GenericSpectrumModel
// -----------------------------
GenericSpectrumModel::GenericSpectrumModel(const TabulatedModelData &table)
    : _lookupTable(table)
{
}

void GenericSpectrumModel::setLookupTable(TabulatedModelData table)
{
    _lookupTable = std::move(table);
}

float GenericSpectrumModel::valueFromModel(float samplePoint, float spacing) const
{
    return _lookupTable.trapezoidIntegral(samplePoint - 0.5f*spacing, samplePoint + 0.5f*spacing);
}

QVariant GenericSpectrumModel::toVariant() const { return QVariant(); }

void GenericSpectrumModel::fromVariant(const QVariant&) { return; }


// _____________________________
// # KramersLawSpectrumModel
// -----------------------------
float KramersLawSpectrumModel::valueFromModel(float samplePoint, float spacing) const
{
    static const float LOW_END = 0.1f;

    float bot = samplePoint - 0.5f*spacing;
    float top = samplePoint + 0.5f*spacing;

    if((top < LOW_END) || (bot > _parameter))
        return 0.0f;

    if(bot < LOW_END)
        bot = LOW_END;
    if(top > _parameter)
        top = _parameter;

    return _parameter * log(top / bot) - (top - bot);
}

QVariant KramersLawSpectrumModel::toVariant() const { return QVariant(); }

void KramersLawSpectrumModel::fromVariant(const QVariant&) { return; }

} // namespace CTL
