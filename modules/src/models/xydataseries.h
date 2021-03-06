#ifndef CTL_XYDATASERIES_H
#define CTL_XYDATASERIES_H

#include "abstractdatamodel.h"
#include "pointseriesbase.h"

namespace CTL {

class XYDataSeries : public PointSeriesBase
{
public:
    enum Sampling { Linear, Exponential };

    XYDataSeries() = default;
    XYDataSeries(QList<QPointF>&& dataSeries);
    XYDataSeries(const QList<QPointF>& dataSeries);
    XYDataSeries(const QVector<float>& x, const QVector<float>& y);

    // factory methods
    static XYDataSeries sampledFromModel(const AbstractDataModel& dataModel,
                                         const QVector<float>& samplingPoints);
    static XYDataSeries sampledFromModel(std::shared_ptr<AbstractDataModel> dataModel,
                                         const QVector<float>& samplingPoints);
    static XYDataSeries sampledFromModel(const AbstractDataModel& dataModel,
                                         const std::vector<float>& samplingPoints);
    static XYDataSeries sampledFromModel(std::shared_ptr<AbstractDataModel> dataModel,
                                         const std::vector<float>& samplingPoints);
    static XYDataSeries sampledFromModel(const AbstractDataModel& dataModel,
                                         float from,
                                         float to,
                                         uint nbSamples,
                                         Sampling samplingPattern = Linear);
    static XYDataSeries sampledFromModel(std::shared_ptr<AbstractDataModel> dataModel,
                                         float from,
                                         float to,
                                         uint nbSamples,
                                         Sampling samplingPattern = Linear);

    // setter methods
    void append(const QPointF& sample);
    void append(float x, float y);
    void append(const QList<QPointF>& series);

    // other methods
    void remove(const QPointF& sample);

    static QVector<float> linSpace(float from, float to, uint nbSamples);
    static QVector<float> expSpace(float from, float to, uint nbSamples);
};

} // namespace CTL

#endif // CTL_XYDATASERIES_H
