#ifndef ABSTRACTDATAMODEL_H
#define ABSTRACTDATAMODEL_H

#include <QVariant>

/*
 * NOTE: This is header only.
 */

namespace CTL {

/*!
 * \class AbstractDataModel
 * \brief The AbstractDataModel class is the base class for basic data models.
 *
 * Sub-classes must implement the method to sample a value at a given position (valueAt()).
 *
 * Parameters can be set by passing a QVariant that contains all necessary information.
 * Re-implement the setParameter() method to parse the QVariant into your required format within
 * sub-classes of AbstractDataModel.
 */
class AbstractDataModel
{
    // abstract interface
    public:virtual QVariant toVariant() const = 0;
    public:virtual void fromVariant(const QVariant& variant) = 0;
    public:virtual float valueAt(float position) const = 0;

public:
    virtual ~AbstractDataModel() = default;

    virtual QVariant parameter() const;
    virtual void setParameter(const QVariant& parameter);
};

/*!
 * \class AbstractDensityDataModel
 * \brief The AbstractDensityDataModel class is the base class for data models that handle density
 * data.
 *
 * Sub-classes must implement the method to sample a value at a given position (valueAt()) and a
 * method that computes the integral of the density function over a given interval (binIntegral()).
 *
 * Parameters can be set by passing a QVariant that contains all necessary information.
 * Re-implement the setParameter() method to parse the QVariant into your required format within
 * sub-classes of AbstractDensityDataModel.
 */
class AbstractDensityDataModel : public AbstractDataModel
{
    // abstract interface
    public:virtual float binIntegral(float position, float binWidth) const = 0;
};


/*!
 * \fn float AbstractDataModel::valueAt(float position) const
 *
 * Returns the value sampled from the model at the given \a position.
 */

/*!
 * \fn QVariant AbstractDataModel::toVariant() const const
 *
 * Encodes all information required to describe this instance into a QVariant.
 * Required to provide de-/serialization functionality.
 */

/*!
 * \fn void AbstractDataModel::fromVariant(const QVariant& variant)
 *
 * Decodes all information required to describe this instance from \a variant.
 * Required to provide de-/serialization functionality.
 */

/*!
 * \fn QVariant AbstractDataModel::parameter() const
 *
 * Returns the parameters of this instance as a QVariant.
 *
 * Re-implement this method within your sub-class such that it encapsulates all necessary
 * information into a QVariant.
 */

/*!
 * \fn void AbstractDataModel::setParameter(const QVariant& parameter);
 *
 * Passes \a parameter to this instance.
 *
 * Encapsulate all necessary information into the passed QVariant and re-implement this method
 * within your sub-class to parse it into your required format.
 */

/*!
 * \fn float AbstractDensityDataModel::binIntegral(float position, float binWidth) const
 *
 * Returns the integral of the density over the interval
 * \f$ \left[position-\frac{binWidth}{2},\,position+\frac{binWidth}{2}\right] \f$.
 */

// implementations
inline QVariant AbstractDataModel::parameter() const
{
    return QVariant();
}

inline void AbstractDataModel::setParameter(const QVariant &)
{
}

} // namespace CTL

#endif // ABSTRACTDATAMODEL_H
