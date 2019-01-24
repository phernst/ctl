#ifndef XYDATASERIES_H
#define XYDATASERIES_H

#include "pointseriesbase.h"
#include "abstractdatamodel.h"

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
    static XYDataSeries sampledFromModel(const AbstractDataModel& dataModel, const QVector<float>& samplingPoints);
    static XYDataSeries sampledFromModel(const AbstractDataModel& dataModel, float from, float to, uint nbSamples, Sampling samplingPattern);

    // setter methods
    void append(const QPointF& sample);
    void append(float x, float y);
    void append(const QList<QPointF>& series);

    // other methods
    void remove(const QPointF& sample);

    static QVector<float> linSpace (float from, float to, uint nbSamples);
    static QVector<float> expSpace (float from, float to, uint nbSamples);
};

inline void XYDataSeries::append(const QPointF &sample) { _data.append(sample); }

inline void XYDataSeries::append(float x, float y) { _data.append(QPointF(x,y)); }

inline void XYDataSeries::append(const QList<QPointF> &series) { _data.append(series); }

inline void XYDataSeries::remove(const QPointF &sample) { _data.removeOne(sample); }


}

#endif // XYDATASERIES_H
