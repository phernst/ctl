#ifndef ABSTRACTDATAMODEL_H
#define ABSTRACTDATAMODEL_H

#include <QVariant>

/*
 * NOTE: This is header only.
 */

namespace CTL {

class AbstractDataModel
{
    // abstract interface
    public:virtual QVariant toVariant() const = 0;
    public:virtual void fromVariant(const QVariant& variant) = 0;
    protected:virtual float valueFromModel(float samplePoint, float spacing) const = 0;

public:
    virtual ~AbstractDataModel() = default;

    virtual float valueAt(float samplePoint, float spacing) const;
};

inline float AbstractDataModel::valueAt(float samplePoint, float spacing) const
{
    return valueFromModel(samplePoint, spacing);
}

} // namespace CTL

#endif // ABSTRACTDATAMODEL_H
