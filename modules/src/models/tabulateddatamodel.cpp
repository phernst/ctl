#include "tabulateddatamodel.h"

namespace CTL {

DECLARE_JSON_COMPATIBLE_TYPE(TabulatedDataModel);

/*!
 * Constructs a TabulatedDataModel with lookup values given by \a table.
 */
TabulatedDataModel::TabulatedDataModel(QMap<float, float> table)
    : _data(std::move(table))
{
}

/*!
 * Constructs a TabulatedDataModel with lookup values given by \a keys and \a values.
 *
 * Data with the same key will be overwritten and the entry occuring last in the vector will remain
 * in the resulting tabulated data.
 *
 * Both vectors need to have the same length; throws an exception otherwise.
 */
TabulatedDataModel::TabulatedDataModel(const QVector<float>& keys, const QVector<float>& values)
{
    setData(keys, values);
}

/*!
 * Returns a constant reference to the lookup table stored in this instance
 */
const QMap<float, float> &TabulatedDataModel::lookupTable() const { return _data; }

/*!
 * Sets the lookup table of this instance to \a table.
 */
void TabulatedDataModel::setData(QMap<float, float> table) { _data = std::move(table); }

/*!
 * Sets the lookup table of this instance to the values given by \a keys and \a values.
 *
 * Data with the same key will be overwritten and the entry occuring last in the vector will remain
 * in the resulting tabulated data.
 *
 * Both vectors need to have the same length; throws an exception otherwise.
 */
void TabulatedDataModel::setData(const QVector<float>& keys, const QVector<float>& values)
{
    Q_ASSERT(keys.size() == values.size());
    if(keys.size() != values.size())
        throw std::domain_error("TabulatedDataModel::setData(): keys and values have different size.");

    _data.clear();
    for(int k = 0; k < keys.length(); ++k)
        _data.insert(keys.at(k), values.at(k));
}

/*!
 * Inserts the (\a key, \a value) pair into the lookup table of this instance.
 *
 * If an entry with the same key already exists, it will be overwritten.
 */
void TabulatedDataModel::insertDataPoint(float key, float value) { _data.insert(key, value); }

QVariantList TabulatedDataModel::dataAsVariantList() const
{
    QVariantList list;

    auto i = _data.constBegin();
    while (i != _data.constEnd())
    {
        list.append(QVariant(QVariantList{i.key(),i.value()}));
        ++i;
    }

    return list;
}

void TabulatedDataModel::setDataFromVariantList(const QVariantList& list)
{
    _data.clear();
    foreach(const QVariant& var, list)
    {
        auto dataPoint = var.toList();
        if(dataPoint.size() < 2)
            continue;
        _data.insert(dataPoint.at(0).toFloat(), dataPoint.at(1).toFloat());
    }
}

/*!
 * Returns the integral of the tabulated data over the interval
 * \f$ \left[position-\frac{binWidth}{2},\,position+\frac{binWidth}{2}\right] \f$.
 *
 * This method uses trapezoid integration. The following figure provides an example of the procedure.
 *
 * \image html TabulatedDataModel.svg "Example of integration of lookup table data in a TabulatedDataModel."
 *
 * The table contains 10 data points (star symbols). Integration is shown for three different cases.
 * Interval 1: The integration bin covers multiple tabulated values. In this case, the result is a
 * sum over all partial intervals. If one (or both) integration border(s) is outside the range of
 * tabulated values, the requested point is extrapolated linearly to zero (see left side of
 * Interval 1). Interval 2: Integration bin lays between two tabulated values. Interval 3: Interval
 * is fully outside the range of tabulated data. This bin integral returns zero.
 */
float TabulatedDataModel::binIntegral(float position, float binWidth) const
{
    float from = position - 0.5f * binWidth;
    float to = position + 0.5f * binWidth;

    float ret = 0.0;

    const auto lowerEndPosition = _data.lowerBound(from);
    const auto upperEndPosition = _data.lowerBound(to);

    // check if integration interval is fully outside tabulated data --> return zero
    if(lowerEndPosition == _data.end() || to < _data.begin().key())
        return 0.0f;

    // integration interval lies fully within two tabulated values --> return value * binWidth
    if(lowerEndPosition == upperEndPosition && upperEndPosition != _data.begin())
        return valueAt(position) * binWidth;

    // if function reaches this point, multiple segments need to be integrated
    // compute contribution of lower end
    float lowerEndValue = valueAt(from);
    float lowerEndContr
        = 0.5f * (lowerEndValue + lowerEndPosition.value()) * (lowerEndPosition.key() - from);

    ret += lowerEndContr;

    qDebug() << "lowerEndContr: Integral from " << lowerEndValue << "(at: " << from << ") to "
             << lowerEndPosition.value() << "(at: " << lowerEndPosition.key()
             << ") = " << lowerEndContr;

    if(upperEndPosition == _data.begin())
        return ret;

    // compute contributions of all 'full segments'
    auto currentPosition = lowerEndPosition;
    while(currentPosition != upperEndPosition)
    {
        if(currentPosition + 1 == _data.end())
            break;
        if((currentPosition + 1).key() > to)
            break;
        float width = (currentPosition + 1).key() - currentPosition.key();
        float height = 0.5f * ((currentPosition + 1).value() + currentPosition.value());

        qDebug() << "contribution for " << currentPosition.key() << "to" << currentPosition.key()
                 << ": " << width << height;

        ret += width * height;
        ++currentPosition;
    }

    if(currentPosition.key() == to)
        return ret;

    // compute contribution of upper end
    const auto lastSample = (upperEndPosition - 1);
    float upperEndValue = valueAt(to);
    float upperEndContr = 0.5f * (lastSample.value() + upperEndValue) * (to - lastSample.key());

    ret += upperEndContr;

    qDebug() << "upperEndContr: Integral from " << (upperEndPosition - 1).value()
             << "(at: " << (upperEndPosition - 1).key() << ") to " << upperEndValue << "(at: " << to
             << ") = " << upperEndContr;

    return ret;
}

AbstractDataModel* TabulatedDataModel::clone() const { return new TabulatedDataModel(*this); }

/*!
 * Returns a linearly interpolated value at position \a pos based on the data in the lookup table.
 *
 * Returns zero if \a pos is outside the range of the available tabulated data.
 */
float TabulatedDataModel::valueAt(float pos) const
{
    // check if data contains an entry for 'pos'
    if(_data.contains(pos))
        return _data.value(pos);

    // find position of next tabulated entry in data with key > pos
    auto nextValidDataPt = _data.lowerBound(pos);

    // check if value is outside of tabulated data --> return 0
    if(nextValidDataPt == _data.begin() || nextValidDataPt == _data.end())
        return 0.0f;

    // now it is assured that pos is contained in range of keys of data
    float weight
        = (nextValidDataPt.key() - pos) / (nextValidDataPt.key() - (nextValidDataPt - 1).key());
    float contribLower = (nextValidDataPt - 1).value() * weight;
    float contribUpper = (nextValidDataPt).value() * (1.0f - weight);

    return contribLower + contribUpper;
}

QVariant TabulatedDataModel::toVariant() const
{
    auto variant = AbstractDataModel::toVariant().toMap();
    variant.insert("data", dataAsVariantList());

    return variant;
}

void TabulatedDataModel::fromVariant(const QVariant& variant)
{
    AbstractDataModel::fromVariant(variant);
    setDataFromVariantList(variant.toMap().value("data").toList());
}


} // namespace CTL
