#ifndef ABSTRACTSPECTRALMODEL_H
#define ABSTRACTSPECTRALMODEL_H

#include "abstractdatamodel.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {

class AbstractSpectralModel : public AbstractDataModel
{
public:
    // Caution: as of now, an override of `valueFromModel` must return semi-positive values only,
    //          otherwise `valueAt` will throw an exception!
    float valueAt(float samplePoint, float spacing) const final;
};

template <typename ParType>
class AbstractParameterizedSpectralModel : public AbstractSpectralModel
{
    // abstract interface
    public:virtual void setParameter(const ParType& parameterToSet) = 0;
    public:virtual const ParType& parameter() const = 0;
};

inline float AbstractSpectralModel::valueAt(float samplePoint, float spacing) const
{
    auto val = valueFromModel(samplePoint, spacing);
    if(val < 0.0f)
        throw std::runtime_error("a negative value is not allowed for AbstractSpectralModel");
    return val;
}

} // namespace CTL

#endif // ABSTRACTSPECTRALMODEL_H
