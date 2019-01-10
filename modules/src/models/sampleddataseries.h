#ifndef SAMPLEDDATASERIES_H
#define SAMPLEDDATASERIES_H

#include "abstractspectralmodel.h"

namespace CTL {

class SampledDataSeries
{
public:
    // Constructors (assessing the data from 'model' with equidistant sampling)
    SampledDataSeries() = default;
    SampledDataSeries(float from, float to, uint nbSamples, const AbstractDataModel& model);

    // Sample points ("x values")
    float samplePoint(uint sampleNb) const;
    std::vector<float> samplePoints() const;

    // Values ("y values")
    float value(uint sampleNb) const;
    const std::vector<float>& values() const;

    // Getter
    float from() const;
    float to() const;
    float spacing() const;
    uint nbSamples() const;

    // modifcation
    void multiplyWith(const std::vector<float>& weights);
    float normalize();

    // convenience methods
    float integral() const; // just the sum
    float integral(const std::vector<float>& weights) const;
    float maxSamplePoint() const;
    SampledDataSeries normalized() const;
    // float positionOfMaxVal() const; // TBD
    QMap<float, float> toMap() const;
    std::map<float, float> toStdMap() const;

    // de-/serializing
    QVariant toVariant() const;
    void fromVariant(const QVariant& variant);

protected:
    float _from;
    float _spacing;
    uint _nbSamples;

    std::vector<float> _sampledData;

private:
    void sampleData(const AbstractDataModel &model);
};

} // namespace CTL

#endif // SAMPLEDDATASERIES_H
