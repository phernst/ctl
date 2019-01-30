#ifndef ABSTRACTDATAMODEL_H
#define ABSTRACTDATAMODEL_H

#include <QVariant>
#include <QDebug>

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

/*!
 * \class AbstractIntegrableDataModel
 * \brief The AbstractIntegrableDataModel class is the base class for data models that provide a
 * means to integrate the contained data.
 *
 * Sub-classes must implement the method to sample a value at a given position (valueAt()) and a
 * method that computes the integral of the data over a given interval (binIntegral()).
 *
 * Parameters can be set by passing a QVariant that contains all necessary information.
 * Re-implement the setParameter() method to parse the QVariant into your required format within
 * sub-classes of AbstractDensityDataModel.
 */

class AbstractDataModel
{
    // abstract interface
    public:virtual float valueAt(float position) const = 0;
    public:virtual AbstractDataModel* clone() const = 0;

public:
    enum { Type = 0, UserType = 65536 };

    virtual ~AbstractDataModel() = default;

    virtual int type() const;
    virtual bool isIntegrable() const final;

    virtual void fromVariant(const QVariant& variant);
    virtual QVariant toVariant() const;
    virtual QVariant parameter() const;
    virtual void setParameter(const QVariant& parameter);

};

class AbstractIntegrableDataModel : public AbstractDataModel
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
 * \fn AbstractDataModel* AbstractDataModel::clone() const
 *
 * Creates a copy of this instance and returns a base class pointer to the new object.
 */

/*!
 * \fn int AbstractDataModel::type() const
 *
 * Returns the type id of this instance.
 */

/*!
 * \fn QVariant AbstractDataModel::toVariant() const
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
 * \fn void AbstractDataModel::setParameter(const QVariant& parameter)
 *
 * Passes \a parameter to this instance.
 *
 * Encapsulate all necessary information into the passed QVariant and re-implement this method
 * within your sub-class to parse it into your required format.
 */

/*!
 * \fn bool AbstractDataModel::isIntegrable() const
 *
 * Returns true if this instance allows integration of data (i.e. it provides the binIntegral()
 * method).
 *
 * Same as dynamic_cast<const AbstractIntegrableDataModel*>(this).
 */

/*!
 * \fn float AbstractIntegrableDataModel::binIntegral(float position, float binWidth) const
 *
 * Returns the integral of the density over the interval
 * \f$ \left[position-\frac{binWidth}{2},\,position+\frac{binWidth}{2}\right] \f$.
 */

/*!
 * \def ADD_TO_MODEL_ENUM(newIndex)
 *
 * Macro to add the model to the model enumeration with the index \a newIndex.
 */
#define ADD_TO_MODEL_ENUM(newIndex)                                                            \
public:                                                                                            \
    enum { Type = newIndex };                                                                      \
    int type() const override { return Type; }                                                     \
                                                                                                   \
private:

// implementations
inline int AbstractDataModel::type() const { return Type; }

inline QVariant AbstractDataModel::parameter() const { return QVariant(); }

inline void AbstractDataModel::setParameter(const QVariant &) { }

inline QVariant AbstractDataModel::toVariant() const
{
    QVariantMap ret;

    ret.insert("type-id", type());
    ret.insert("name", typeid(*this).name());
    ret.insert("parameters", parameter());

    return ret;
}

inline void AbstractDataModel::fromVariant(const QVariant &variant)
{
    auto map = variant.toMap();
    if(map.value("type-id").toInt() != type())
    {
        qWarning() << QString(typeid(*this).name()) + "::fromVariant: Could not construct instance! "
                      "reason: incompatible variant passed";
        return;
    }

    setParameter(map.value("parameters").toMap());
}

inline bool AbstractDataModel::isIntegrable() const
{
    return dynamic_cast<const AbstractIntegrableDataModel*>(this);
}


} // namespace CTL

#endif // ABSTRACTDATAMODEL_H
