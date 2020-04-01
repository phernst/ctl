#include "intervaldataseries.h"

namespace CTL {

IntervalDataSeries IntervalDataSeries::sampledFromModel(const AbstractIntegrableDataModel& dataModel,
                                                        float from, float to, uint nbSamples)
{
    Q_ASSERT(from <= to);
    Q_ASSERT(nbSamples > 0);

    IntervalDataSeries ret;
    ret._binWidth = (to - from) / float(nbSamples);

    float smpPt = from + 0.5f * ret._binWidth;
    for(uint s = 0; s < nbSamples; ++s)
    {
        ret._data.append(QPointF(smpPt, dataModel.binIntegral(smpPt, ret._binWidth)));
        smpPt += ret._binWidth;
    }

    return ret;
}

IntervalDataSeries IntervalDataSeries::sampledFromModel(
        std::shared_ptr<AbstractIntegrableDataModel> dataModel, float from, float to, uint nbSamples)
{
    return sampledFromModel(*dataModel, from, to, nbSamples);
}

float IntervalDataSeries::integral() const
{
    return std::accumulate(
        _data.constBegin(), _data.constEnd(), 0.0f,
        [](float val, const QPointF& p2) { return val + float(p2.y()); });
}

float IntervalDataSeries::integral(const std::vector<float>& weights) const
{
    Q_ASSERT(weights.size() == nbSamples());

    return std::inner_product(
        _data.constBegin(), _data.constEnd(), weights.cbegin(), 0.0f,
        [](float a, float b) { return a + b; },
        [](const QPointF& p, float w) { return float(p.y()) * w; });
}

void IntervalDataSeries::normalizeByIntegral()
{
    const auto intgr = integral();

    if(qFuzzyIsNull(intgr))
    {
        qWarning("Trying to normalize data series with integral 0. Skipped normalization.");
        return;
    }

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

float IntervalDataSeries::centroid() const
{
    float sum = 0.0f;

    for(const auto& pt : _data)
        sum += float(pt.x()) * float(pt.y());

    return sum / integral();
}

SamplingRange IntervalDataSeries::samplingRange() const
{
    const auto halfBinWidth = 0.5f * _binWidth;
    return { _data.front().x() - halfBinWidth, _data.back().x() + halfBinWidth };
}

void IntervalDataSeries::clampToRange(const SamplingRange& range)
{
    const auto halfBinWidth = 0.5f * _binWidth;

    auto isFullyContained = [&range, halfBinWidth] (float binPos)
    {
        const auto binStart = binPos - halfBinWidth;
        const auto binEnd = binPos + halfBinWidth;
        return binStart >= range.start() && binEnd <= range.end();
    };

    auto isFullyOutside = [&range, halfBinWidth] (float binPos)
    {
        const auto binStart = binPos - halfBinWidth;
        const auto binEnd = binPos + halfBinWidth;
        return binEnd < range.start() || binStart > range.end();
    };

    auto relBinOverlap = [&range, halfBinWidth] (float binPos)
    {
        const auto binStart = binPos - halfBinWidth;
        const auto binEnd = binPos + halfBinWidth;
        const auto absoluteOverlap = std::min(binEnd, range.end()) -
                                     std::max(binStart, range.start());
        return absoluteOverlap / (2.0f * halfBinWidth);
    };

    for(auto& pt : _data)
    {
        const auto binPos = float(pt.x());

        if(isFullyContained(binPos))
            continue;
        else if(isFullyOutside(binPos))
            pt.ry() = 0.0;
        else
            pt.ry() = pt.y() * relBinOverlap(binPos);
    }
}

} // namespace CTL
