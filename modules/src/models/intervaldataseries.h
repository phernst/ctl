#ifndef CTL_INTERVALDATASERIES_H
#define CTL_INTERVALDATASERIES_H

#include "pointseriesbase.h"
#include "abstractdatamodel.h"
#include "processing/coordinates.h"

namespace CTL {

class IntervalDataSeries : public PointSeriesBase
{
public:
    IntervalDataSeries() = default;

    // factory methods
    static IntervalDataSeries sampledFromModel(const AbstractIntegrableDataModel& dataModel,
                                               float from, float to, uint nbSamples);
    static IntervalDataSeries sampledFromModel(std::shared_ptr<AbstractIntegrableDataModel> dataModel,
                                               float from, float to, uint nbSamples);

    // other methods
    float integral() const;
    float integral(const std::vector<float>& weights) const;
    void normalizeByIntegral();
    IntervalDataSeries normalizedByIntegral() const;
    float binWidth() const;
    float centroid() const;
    SamplingRange samplingRange() const;
    void clampToRange(const SamplingRange& range);

private:
    float _binWidth{};
};

}
#endif // CTL_INTERVALDATASERIES_H
