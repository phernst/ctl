#ifndef XYDATASERIES_H
#define XYDATASERIES_H

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
    static XYDataSeries sampledFromModel(const AbstractDataModel& dataModel,
                                         float from,
                                         float to,
                                         uint nbSamples,
                                         Sampling samplingPattern);

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

#endif // XYDATASERIES_H
