#include "sampleddataseries.h"

namespace CTL {

SampledDataSeries::SampledDataSeries(float from,
                                         float to,
                                         uint nbSamples,
                                         const AbstractDataModel &model)
    : _from(from)
    , _spacing((to - from) / float(nbSamples))
    , _nbSamples(nbSamples)
{
    Q_ASSERT(from <= to);
    Q_ASSERT(nbSamples > 0);

    sampleData(model);
}

void SampledDataSeries::sampleData(const AbstractDataModel& model)
{
    // allocate memory
    _sampledData.resize(_nbSamples);
    // get data from spectral model
    for(uint smp = 0; smp < _nbSamples; ++smp)
    {
        _sampledData[smp] = model.valueAt(samplePoint(smp), _spacing);
        Q_ASSERT_X(_sampledData[smp], "sample spectral data", "sampled value is negative");
    }
}

float SampledDataSeries::samplePoint(uint sampleNb) const
{
    Q_ASSERT(sampleNb < _nbSamples);
    return _from + (float(sampleNb) + 0.5f) * _spacing;
}

std::vector<float> SampledDataSeries::samplePoints() const
{
    std::vector<float> ret(_nbSamples);
    for(uint i = 0; i < _nbSamples; ++i)
        ret[i] = samplePoint(i);
    return ret;
}

float SampledDataSeries::value(uint sampleNb) const
{
    Q_ASSERT(sampleNb < _nbSamples);
    return _sampledData[sampleNb];
}

const std::vector<float>& SampledDataSeries::values() const { return _sampledData; }

QMap<float, float> SampledDataSeries::toMap() const
{
    QMap<float, float> ret;
    auto keys = samplePoints();
    for(uint i = 0; i < _nbSamples; ++i)
        ret.insert(keys[i], _sampledData[i]);
    return ret;
}

std::map<float, float> SampledDataSeries::toStdMap() const
{
    std::map<float, float> ret;
    auto keys = samplePoints();
    for(uint i = 0; i < _nbSamples; ++i)
        ret[keys[i]] = _sampledData[i];
    return ret;
}

float SampledDataSeries::from() const { return _from; }

float SampledDataSeries::to() const { return _from + _nbSamples * _spacing; }

float SampledDataSeries::spacing() const { return _spacing; }

uint SampledDataSeries::nbSamples() const { return _nbSamples; }

float SampledDataSeries::integral() const
{
    float sum = 0.0f;
    for(const auto& val : _sampledData)
        sum += val;
    return sum;
}

float SampledDataSeries::integral(const std::vector<float>& weights) const
{
    Q_ASSERT(weights.size() == _nbSamples);
    float sum = 0.0f;
    for(uint i = 0; i < _nbSamples; ++i)
        sum += _sampledData[i] * weights[i];
    return sum;
}

float SampledDataSeries::maxSamplePoint() const
{
    return _from + (float(_nbSamples) - 0.5f) * _spacing;
}

SampledDataSeries SampledDataSeries::normalized() const
{
    SampledDataSeries ret(*this);
    ret.normalize();
    return ret;
}

void SampledDataSeries::multiplyWith(const std::vector<float> &weights)
{
    Q_ASSERT(weights.size() == _nbSamples);

    for(uint i = 0; i < _nbSamples; ++i)
        _sampledData[i] *= weights[i];
}

float SampledDataSeries::normalize()
{
    // normalize to integral = 1
    auto total = integral();
    if(qFuzzyIsNull(total))
        throw std::runtime_error("normalize(): total spectral intensity is close to zero");

    for(auto& val : _sampledData)
        val /= total;

    return total;
}

QVariant SampledDataSeries::toVariant() const
{
    QVariantMap map;
    map.insert("from", _from);
    map.insert("spacing", _spacing);
    map.insert("nb samples", _nbSamples);
    QVariantList samplePts;
    for(const auto& val : _sampledData)
        samplePts.append(val);
    map.insert("values", samplePts);

    return map;
}

void SampledDataSeries::fromVariant(const QVariant& variant)
{
    QVariantMap map = variant.toMap();
    _from = map.value("from").toFloat();
    _spacing = map.value("spacing").toFloat();
    _nbSamples = map.value("nb samples").toUInt();
    _sampledData.resize(_nbSamples);

    QVariantList samplePts = map.value("values").toList();
    Q_ASSERT_X(uint(samplePts.length()) == _nbSamples, "spectral data from variant",
               "inconsistent dimensions in variant");
    for(uint i = 0; i < _nbSamples; ++i)
        _sampledData[i] = samplePts.at(i).toFloat();
}

}
