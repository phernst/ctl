#ifndef ABSTRACTDATAMODEL_H
#define ABSTRACTDATAMODEL_H

#include "io/serializationinterface.h"
#include "copyableuniqueptr.h"

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

// Data model operators
std::shared_ptr<AbstractDataModel> operator+(std::shared_ptr<AbstractDataModel> lhs,
                                             std::shared_ptr<AbstractDataModel> rhs);
std::shared_ptr<AbstractDataModel> operator-(std::shared_ptr<AbstractDataModel> lhs,
                                             std::shared_ptr<AbstractDataModel> rhs);
std::shared_ptr<AbstractDataModel> operator*(std::shared_ptr<AbstractDataModel> lhs,
                                             std::shared_ptr<AbstractDataModel> rhs);
std::shared_ptr<AbstractDataModel> operator/(std::shared_ptr<AbstractDataModel> lhs,
                                             std::shared_ptr<AbstractDataModel> rhs);
std::shared_ptr<AbstractDataModel> operator|(std::shared_ptr<AbstractDataModel> lhs,
                                             std::shared_ptr<AbstractDataModel> rhs);
std::shared_ptr<AbstractDataModel>& operator+=(std::shared_ptr<AbstractDataModel>& lhs,
                                               const std::shared_ptr<AbstractDataModel>& rhs);
std::shared_ptr<AbstractDataModel>& operator-=(std::shared_ptr<AbstractDataModel>& lhs,
                                               const std::shared_ptr<AbstractDataModel>& rhs);
std::shared_ptr<AbstractDataModel>& operator*=(std::shared_ptr<AbstractDataModel>& lhs,
                                               const std::shared_ptr<AbstractDataModel>& rhs);
std::shared_ptr<AbstractDataModel>& operator/=(std::shared_ptr<AbstractDataModel>& lhs,
                                               const std::shared_ptr<AbstractDataModel>& rhs);
std::shared_ptr<AbstractDataModel>& operator|=(std::shared_ptr<AbstractDataModel>& lhs,
                                               const std::shared_ptr<AbstractDataModel>& rhs);

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

// Declare DataModelPtr (copyable unique_ptr)
template <class ModelType, class =
          typename std::enable_if<std::is_convertible<ModelType*, AbstractDataModel*>::value>::type>
using DataModelPtr = CopyableUniquePtr<ModelType>;

using AbstractDataModelPtr = DataModelPtr<AbstractDataModel>;

// factory function `makeDataModel`
template <typename ModelType, typename... ConstructorArguments>
auto makeDataModel(ConstructorArguments&&... arguments) ->
    typename std::enable_if<std::is_convertible<ModelType*, AbstractDataModel*>::value,
                            std::unique_ptr<ModelType>>::type
{
    return std::unique_ptr<ModelType>(
        new ModelType(std::forward<ConstructorArguments>(arguments)...));
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
