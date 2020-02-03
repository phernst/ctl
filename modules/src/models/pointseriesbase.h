#ifndef POINTSERIESBASE_H
#define POINTSERIESBASE_H

#include <QList>
#include <QPair>
#include <QPointF>
#include <vector>

namespace CTL {

class PointSeriesBase
{
public:
    // getter methods
    QList<QPointF>& data();
    const QList<QPointF>& data() const;

    // other methods
    double max() const;
    double min() const;
    uint nbSamples() const;
    void normalizeByMaxAbsVal();
    void normalizeByMaxVal();
    QPair<double,double> yRange() const;
    void scale(float factor);
    uint size() const;
    //void multiplyWith(const std::vector<float>& weights);

    // Sampling points ("x values")
    float samplingPoint(uint sampleNb) const;
    std::vector<float> samplingPoints() const;

    // Values ("y values")
    float value(uint sampleNb) const;
    std::vector<float> values() const;

protected:
    // ctors
    PointSeriesBase() = default;
    explicit PointSeriesBase(const QList<QPointF>& pointSeries);
    explicit PointSeriesBase(QList<QPointF>&& pointSeries);

    QList<QPointF> _data;
};

inline PointSeriesBase::PointSeriesBase(const QList<QPointF>& pointSeries)
    : _data(pointSeries)
{
}

inline PointSeriesBase::PointSeriesBase(QList<QPointF>&& pointSeries)
    : _data(std::move(pointSeries))
{
}

inline QList<QPointF>& PointSeriesBase::data()
{
    return _data;
}

inline const QList<QPointF>& PointSeriesBase::data() const
{
    return _data;
}

inline double PointSeriesBase::max() const
{
    auto maxEl = std::max_element(_data.begin(), _data.end(),
                                  [] (const QPointF& a, const QPointF& b) { return a.y()<b.y(); });

    return maxEl->y();
}

inline double PointSeriesBase::min() const
{
    auto minEl = std::min_element(_data.begin(), _data.end(),
                                  [] (const QPointF& a, const QPointF& b) { return a.y()<b.y(); });

    return minEl->y();
}

inline uint PointSeriesBase::nbSamples() const
{
    return static_cast<uint>(_data.size());
}

inline void PointSeriesBase::normalizeByMaxAbsVal()
{
    auto maxEl = std::max_element(_data.begin(), _data.end(),
                                  [] (const QPointF& a, const QPointF& b) {
                                  return qAbs(a.y()) < qAbs(b.y()); });
    scale(1.0f / maxEl->y());
}

inline void PointSeriesBase::normalizeByMaxVal()
{
    auto maxEl = std::max_element(_data.begin(), _data.end(),
                                  [] (const QPointF& a, const QPointF& b) { return a.y()<b.y(); });
    scale(1.0f / maxEl->y());
}

inline QPair<double, double> PointSeriesBase::yRange() const
{
    return qMakePair(this->min(), this->max());
}

inline void PointSeriesBase::scale(float factor)
{
    for(auto& pt : _data)
        pt.ry() *= factor;
}

inline uint PointSeriesBase::size() const
{
    return static_cast<uint>(_data.size());
}

inline float PointSeriesBase::samplingPoint(uint sampleNb) const
{
    return _data.at(sampleNb).x();
}

inline std::vector<float> PointSeriesBase::samplingPoints() const
{
    std::vector<float> ret(nbSamples());
    std::transform(_data.begin(), _data.end(), ret.begin(),
                   [](const QPointF& pt) -> float { return pt.x(); });
    return ret;
}

inline float PointSeriesBase::value(uint sampleNb) const
{
    return _data.at(sampleNb).y();
}

inline std::vector<float> PointSeriesBase::values() const
{
    std::vector<float> ret(nbSamples());
    std::transform(_data.begin(), _data.end(), ret.begin(),
                   [](const QPointF& pt) -> float { return pt.y(); });
    return ret;
}

} // namespace CTL

#endif // POINTSERIESBASE_H
