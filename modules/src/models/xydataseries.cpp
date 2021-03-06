#include "xydataseries.h"
#include <QVector>
#include <cmath>

namespace CTL {

XYDataSeries::XYDataSeries(QList<QPointF> &&dataSeries)
    : PointSeriesBase(std::move(dataSeries))
{
}

XYDataSeries::XYDataSeries(const QList<QPointF> &dataSeries)
    : PointSeriesBase(dataSeries)
{
}

XYDataSeries::XYDataSeries(const QVector<float>& x, const QVector<float>& y)
{
    Q_ASSERT(x.size() == y.size());
    if(x.size() != y.size())
        throw std::domain_error("XYDataSeries(const QVector<float> &samplingPoints, const QVector<float> &values): "
                                "Vector of sampling points has different size than value vector");

    const uint nbSmpPts = x.size();
    _data.reserve(nbSmpPts);
    for(uint smpPt = 0; smpPt<nbSmpPts; ++smpPt)
        append(QPointF(x.at(smpPt), y.at(smpPt)));
}

XYDataSeries XYDataSeries::sampledFromModel(const AbstractDataModel& dataModel,
                                            const QVector<float>& samplingPoints)
{
    XYDataSeries ret;

    for(auto smpPt : samplingPoints)
        ret.append(QPointF(smpPt, dataModel.valueAt(smpPt)));

    return ret;
}

XYDataSeries XYDataSeries::sampledFromModel(std::shared_ptr<AbstractDataModel> dataModel,
                                            const QVector<float>& samplingPoints)
{
    return sampledFromModel(*dataModel, samplingPoints);
}

XYDataSeries XYDataSeries::sampledFromModel(const AbstractDataModel& dataModel,
                                            const std::vector<float>& samplingPoints)
{
    XYDataSeries ret;

    for(auto smpPt : samplingPoints)
        ret.append(QPointF(smpPt, dataModel.valueAt(smpPt)));

    return ret;
}

XYDataSeries XYDataSeries::sampledFromModel(std::shared_ptr<AbstractDataModel> dataModel,
                                            const std::vector<float>& samplingPoints)
{
    return sampledFromModel(*dataModel, samplingPoints);
}

XYDataSeries XYDataSeries::sampledFromModel(const AbstractDataModel& dataModel, float from,
                                            float to, uint nbSamples, Sampling samplingPattern)
{
    switch (samplingPattern) {
    case Linear:
        return sampledFromModel(dataModel, linSpace(from, to, nbSamples));
    case Exponential:
        return sampledFromModel(dataModel, expSpace(from, to, nbSamples));
    }

    return XYDataSeries();
}

XYDataSeries XYDataSeries::sampledFromModel(std::shared_ptr<AbstractDataModel> dataModel, float from,
                                            float to, uint nbSamples, Sampling samplingPattern)
{
    return sampledFromModel(*dataModel, from, to, nbSamples, samplingPattern);
}

QVector<float> XYDataSeries::linSpace(float from, float to, uint nbSamples)
{
    QVector<float> ret(nbSamples);
    const float increment = (nbSamples > 1) ? (to - from) / float(nbSamples - 1)
                                            : 0.0f;
    float val = from;
    for(auto& smp : ret)
    {
        smp = val;
        val += increment;
    }
    return ret;

    /* std version
    float val = from-increment;
    std::generate(ret.begin(), ret.end(), [&val, increment]{ return val+=increment; });
    return ret;
    */
}

QVector<float> XYDataSeries::expSpace(float from, float to, uint nbSamples)
{
    Q_ASSERT(from > 0.0f);
    if(from <= 0.0f)
        throw std::domain_error("Exponential sampling is not supported for negative sampling points");

    QVector<float> ret = linSpace(std::log(from), std::log(to), nbSamples);
    for(auto& smp : ret)
        smp = std::exp(smp);
    return ret;
}

void XYDataSeries::append(const QPointF &sample) { _data.append(sample); }

void XYDataSeries::append(float x, float y) { _data.append(QPointF(x,y)); }

void XYDataSeries::append(const QList<QPointF> &series) { _data.append(series); }

void XYDataSeries::remove(const QPointF &sample) { _data.removeOne(sample); }

} // namespace CTL
