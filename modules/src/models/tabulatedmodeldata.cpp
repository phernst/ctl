#include "tabulatedmodeldata.h"

#include <QDebug>

namespace CTL {

TabulatedModelData::TabulatedModelData(const QMap<float, float>& table)
    : _data(table)
{
}

void TabulatedModelData::setData(QMap<float, float> tables) { _data = std::move(tables); }

float TabulatedModelData::trapezoidIntegral(float from, float to) const
{
    float ret = 0.0;

    const auto lowerEndPosition = _data.lowerBound(from);
    const auto upperEndPosition = _data.lowerBound(to);

    // check if integration interval is fully outside tabulated data
    if(lowerEndPosition == _data.end() || to < _data.begin().key())
        return 0.0f;

    // integration interval lies fully within two tabulated values
    if(lowerEndPosition == upperEndPosition && upperEndPosition != _data.begin())
        return interpLin(0.5f * (from + to)) * (to - from);

    // compute contribution of lower end
    float lowerEndValue = interpLin(from);
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
    float upperEndValue = interpLin(to);
    float upperEndContr = 0.5f * (lastSample.value() + upperEndValue) * (to - lastSample.key());

    ret += upperEndContr;

    qDebug() << "upperEndContr: Integral from " << (upperEndPosition - 1).value()
             << "(at: " << (upperEndPosition - 1).key() << ") to " << upperEndValue << "(at: " << to
             << ") = " << upperEndContr;

    return ret;
}

float TabulatedModelData::interpLin(float pos) const
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

} // namespace CTL
