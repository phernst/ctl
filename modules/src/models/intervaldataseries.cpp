#include "intervaldataseries.h"

namespace CTL {

IntervalDataSeries::IntervalDataSeries()
{

}

IntervalDataSeries IntervalDataSeries::sampledFromModel(const AbstractIntegrableDataModel &dataModel, float from, float to, uint nbSamples)
{
    Q_ASSERT(from <= to);
    Q_ASSERT(nbSamples > 0);

    IntervalDataSeries ret;
    ret._binWidth = ((to - from) / float(nbSamples));

    float smpPt = from + 0.5f * ret._binWidth;
    for(uint s = 0; s<nbSamples; ++s)
    {
        ret._data.append(QPointF(smpPt, dataModel.binIntegral(smpPt, ret._binWidth)));
        smpPt += ret._binWidth;
    }

    return ret;
}

float IntervalDataSeries::integral() const
{
    float sum = 0.0f;
    for(const auto& pt : _data)
        sum += pt.y();
    return sum;
}

float IntervalDataSeries::integral(const std::vector<float>& weights) const
{
    Q_ASSERT(weights.size() == nbSamples());
    float sum = 0.0f;
    for(uint i = 0; i < nbSamples(); ++i)
        sum += _data.at(i).y() * weights[i];
    return sum;
}

void IntervalDataSeries::normalizeByIntegral()
{
    auto intgr = integral();
    for(auto& pt : _data)
        pt.ry() /= intgr;
}

IntervalDataSeries IntervalDataSeries::normalizedByIntegral() const
{
    IntervalDataSeries ret(*this);
    ret.normalizeByIntegral();
    return ret;
}

float IntervalDataSeries::binWidth() const
{
    return _binWidth;
}



}
