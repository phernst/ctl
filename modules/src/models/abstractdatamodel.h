#ifndef ABSTRACTDATAMODEL_H
#define ABSTRACTDATAMODEL_H

#include "io/serializationinterface.h"
#include <QDebug>
#include <memory>

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
 *
 * To enable de-/serialization if sub-class objects, also reimplement the parameter() method such
 * that it stores all parameters of the model in a QVariant. Additionally, call the macro
 * #DECLARE_SERIALIZABLE_TYPE(YourNewClassName) within the .cpp file of your new class (substitute
 * "YourNewClassName" with the actual class name). Objects of the new class can then be
 * de-/serialized with any of the serializer classes (see also AbstractSerializer).
 *
 * Alternatively, the de-/serialization interface methods toVariant() and fromVariant() can be
 * reimplemented directly. This might be required in some specific situations (usually when
 * polymorphic class members are in use). For the majority of cases, using the parameter() /
 * setParameter() approach should be sufficient and is recommended.
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
 * sub-classes of AbstractIntegrableDataModel.
 *
 * To enable de-/serialization if sub-class objects, also reimplement the parameter() method such
 * that it stores all parameters of the model in a QVariant. Additionally, call the macro
 * #DECLARE_SERIALIZABLE_TYPE(YourNewClassName) within the .cpp file of your new class (substitute
 * "YourNewClassName" with the actual class name). Objects of the new class can then be
 * de-/serialized with any of the serializer classes (see also AbstractSerializer).
 *
 * Alternatively, the de-/serialization interface methods toVariant() and fromVariant() can be
 * reimplemented directly. This might be required in some specific situations (usually when
 * polymorphic class members are in use). For the majority of cases, using the parameter() /
 * setParameter() approach should be sufficient and is recommended.
 */

/*!
 * \class DataModelPtr
 * \brief Helper class to provide unique_ptr like behavior for subclasses of AbstractDataModel with
 * option to copy by means of deep copying (provided by clone()).
 */

class AbstractDataModel : public SerializationInterface
{
    CTL_TYPE_ID(0)

    // abstract interface
    public:virtual float valueAt(float position) const = 0;
    public:virtual AbstractDataModel* clone() const = 0;

public:
    virtual bool isIntegrable() const final;
    virtual QVariant parameter() const;
    virtual void setParameter(const QVariant& parameter);

    void fromVariant(const QVariant& variant) override;
    QVariant toVariant() const override;

    void setName(const QString& name);
    const QString& name() const;

    ~AbstractDataModel() override = default;

private:
    QString _name;

protected:
    AbstractDataModel() = default;
    AbstractDataModel(const AbstractDataModel&) = default;
    AbstractDataModel(AbstractDataModel&&) = default;
    AbstractDataModel& operator=(const AbstractDataModel&) = default;
    AbstractDataModel& operator=(AbstractDataModel&&) = default;
};

class AbstractIntegrableDataModel : public AbstractDataModel
{
    // abstract interface
    public:virtual float binIntegral(float position, float binWidth) const = 0;

public:
    ~AbstractIntegrableDataModel() override = default;

protected:
    AbstractIntegrableDataModel() = default;
    AbstractIntegrableDataModel(const AbstractIntegrableDataModel&) = default;
    AbstractIntegrableDataModel(AbstractIntegrableDataModel&&) = default;
    AbstractIntegrableDataModel& operator=(const AbstractIntegrableDataModel&) = default;
    AbstractIntegrableDataModel& operator=(AbstractIntegrableDataModel&&) = default;
};

struct DataModelPtr
{
    DataModelPtr(std::unique_ptr<AbstractDataModel> model = nullptr);
    DataModelPtr(const DataModelPtr& other);
    DataModelPtr(DataModelPtr&& other) = default;
    DataModelPtr& operator=(const DataModelPtr& other);
    DataModelPtr& operator=(DataModelPtr&& other) = default;
    ~DataModelPtr() = default;

    AbstractDataModel* operator->() const;
    AbstractDataModel& operator*() const;

    AbstractDataModel* get() const;
    void reset(AbstractDataModel* model = nullptr);

    std::unique_ptr<AbstractDataModel> ptr;
};

// factory function `makeDataModel`
template <typename ModelType, typename... ConstructorArguments>
auto makeDataModel(ConstructorArguments&&... arguments) ->
    typename std::enable_if<std::is_convertible<ModelType*, AbstractDataModel*>::value,
                            std::unique_ptr<ModelType>>::type
{
    return std::unique_ptr<ModelType>(
        new ModelType(std::forward<ConstructorArguments>(arguments)...));
}

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
 *
 * Best practice is to invoke the base class version of this method to take care
 * of all content originating from underlying base classes.
 *
 * A typical reimplementation in sub-classes might look like this:
 * \code
 * QVariantMap ret = DirectBaseClass::parameter().toMap();
 *
 * ret.insert("my new parameter", _myNewParameter);
 *
 * return ret;
 * \endcode
 */

/*!
 * \fn void AbstractDataModel::setParameter(const QVariant& parameter)
 *
 * Passes \a parameter to this instance.
 *
 * Encapsulate all necessary information into the passed QVariant and re-implement this method
 * within your sub-class to parse it into your required format.
 *
 * Best practice is to invoke the base class version of this method to take care
 * of all content originating from underlying base classes.
 *
 * A typical reimplementation in sub-classes might look like this:
 * \code
 * DirectBaseClass::setParameter(parameter);
 *
 * // assuming our class has a parameter member "double _myNewParameter"
 *
 * _myNewParameter = parameter.toMap().value("my new parameter").toDouble();
 * \endcode
 */

/*!
* \fn void AbstractDataModel::setName(const QString& name)
*
* Sets the name of this model to \a name.
*/

/*!
* \fn const QString& AbstractDataModel::name() const
*
* Returns the name of this model.
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

// implementations
inline QVariant AbstractDataModel::parameter() const { return QVariant(); }

inline void AbstractDataModel::setParameter(const QVariant&) {}

inline void AbstractDataModel::setName(const QString &name) { _name = name; }

inline const QString& AbstractDataModel::name() const { return _name; }

// Use SerializationInterface::toVariant() documentation.
inline QVariant AbstractDataModel::toVariant() const
{
    QVariantMap ret = SerializationInterface::toVariant().toMap();

    QString nameString = _name.isEmpty() ? typeid(*this).name()
                                         : _name;
    ret.insert("name", nameString);
    ret.insert("parameters", parameter());

    return ret;
}

// Use SerializationInterface::fromVariant() documentation.
inline void AbstractDataModel::fromVariant(const QVariant& variant)
{
    auto map = variant.toMap();
    if(map.value("type-id").toInt() != type())
    {
        qWarning() << QString(typeid(*this).name())
                + "::fromVariant: Could not construct instance! "
                  "reason: incompatible variant passed";
        return;
    }

    _name = map.value("name").toString();
    setParameter(map.value("parameters").toMap());
}

inline bool AbstractDataModel::isIntegrable() const
{
    return dynamic_cast<const AbstractIntegrableDataModel*>(this);
}

inline DataModelPtr::DataModelPtr(std::unique_ptr<AbstractDataModel> model)
    : ptr(std::move(model))
{
}

inline DataModelPtr::DataModelPtr(const DataModelPtr& other)
    : ptr(other.ptr ? other->clone() : nullptr)
{
}

inline DataModelPtr& DataModelPtr::operator=(const DataModelPtr &other)
{
    ptr.reset(other.ptr ? other->clone() : nullptr);
        return *this;
}

inline AbstractDataModel* DataModelPtr::operator->() const
{
    Q_ASSERT(ptr);
    return ptr.get();
}

inline AbstractDataModel& DataModelPtr::operator*() const
{
    Q_ASSERT(ptr);
    return *ptr;
}

inline AbstractDataModel* DataModelPtr::get() const
{
    return ptr.get();
}

inline void DataModelPtr::reset(AbstractDataModel* model)
{
    ptr.reset(model);
}

} // namespace CTL

/*! \file */
///@{
/*!
* \fn std::unique_ptr<ModelType> CTL::makeDataModel(ConstructorArguments&&... arguments)
* \relates AbstractDataModel
* Global (free) make function that creates a new AbstractDataModel from the constructor \a arguments.
* The component is returned as a `std::unique_ptr<ModelType>`, whereas `ModelType` is the
* template argument of this function that needs to be specified.
*/
///@}


#endif // ABSTRACTDATAMODEL_H
