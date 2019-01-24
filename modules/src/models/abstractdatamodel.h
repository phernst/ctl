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
    public:virtual float valueAt(float position) const = 0;

public:
    virtual ~AbstractDataModel() = default;

    inline virtual QVariant parameter() const;
    inline virtual void setParameter(QVariant parameter);
};

class AbstractDensityDataModel : public AbstractDataModel
{
    // abstract interface
    public:virtual float binIntegral(float position, float binWidth) const = 0;
};

QVariant AbstractDataModel::parameter() const
{
    return QVariant();
}

void AbstractDataModel::setParameter(QVariant)
{
}

} // namespace CTL

#endif // ABSTRACTDATAMODEL_H
