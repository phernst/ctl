#ifndef INTERVALDATASERIES_H
#define INTERVALDATASERIES_H

#include "pointseriesbase.h"
#include "abstractdatamodel.h"

namespace CTL {

class IntervalDataSeries : public PointSeriesBase
{
public:
    IntervalDataSeries() = default;

    // factory methods
    static IntervalDataSeries sampledFromModel(const AbstractIntegrableDataModel& dataModel,
                                               float from, float to, uint nbSamples);

    // other methods
    float integral() const;
    float integral(const std::vector<float>& weights) const;
    void normalizeByIntegral();
    IntervalDataSeries normalizedByIntegral() const;
    float binWidth() const;
    float centroid() const;

private:
    float _binWidth{};
};

}
#endif // INTERVALDATASERIES_H
