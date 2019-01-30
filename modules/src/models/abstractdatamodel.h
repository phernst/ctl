#ifndef ABSTRACTDATAMODEL_H
#define ABSTRACTDATAMODEL_H

#include "jsonmodelparser.h"
#include <QDebug>
#include <QVariant>
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

/*!
 * \class DataModelPtr
 * \brief Helper class to provide unique_ptr like behavior for subclasses of AbstractDataModel with
 * option to copy by means of deep copying (provided by clone()).
 */

class AbstractDataModel
{
    // abstract interface
    public:virtual float valueAt(float position) const = 0;
    public:virtual AbstractDataModel* clone() const = 0;

public:
    enum { Type = 0, UserType = 65536 };

    template <class ModelType>
    struct RegisterToJsonParser
    {
        RegisterToJsonParser()
        {
            static auto factoryFunction = [](const QJsonObject& json) {
                return std::unique_ptr<AbstractDataModel>(new ModelType(json));
            };
            JsonModelParser::instance().modelFactories().insert(ModelType::Type, factoryFunction);
        }
    };

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

struct DataModelPtr
{
    DataModelPtr(std::unique_ptr<AbstractDataModel> model = nullptr);
    DataModelPtr(const DataModelPtr& other);
    DataModelPtr(DataModelPtr&& other) = default;
    DataModelPtr& operator=(const DataModelPtr& other);
    DataModelPtr& operator=(DataModelPtr&& other) = default;

    std::unique_ptr<AbstractDataModel> ptr;
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
#define ADD_TO_MODEL_ENUM(newIndex)                                                                \
public:                                                                                            \
    enum { Type = newIndex };                                                                      \
    int type() const override { return Type; }                                                     \
                                                                                                   \
private:

/*!
 * \def REGISTER_TO_JSON_MODEL_PARSER(dataModelClassName_woNamespace)
 *
 * Declares a global variable for a certain data model. Its initialization registers this data model
 * to the JsonModelParser. The argument of this macro must be the name of the concrete data model
 * that should be registered. The name must not contain any namespace, which can be achieved by
 * using this macro inside the according namespace.
 *
 * The name of the globar variable is `JSON_MODEL_PARSER_KNOWS_<dataModelClassName_woNamespace>`.
 */
#define REGISTER_TO_JSON_MODEL_PARSER(dataModelClassName_woNamespace)                              \
    CTL::AbstractDataModel::RegisterToJsonParser<dataModelClassName_woNamespace>                   \
    JSON_MODEL_PARSER_KNOWS_ ## dataModelClassName_woNamespace;

// implementations
inline int AbstractDataModel::type() const { return Type; }

inline QVariant AbstractDataModel::parameter() const { return QVariant(); }

inline void AbstractDataModel::setParameter(const QVariant&) {}

inline QVariant AbstractDataModel::toVariant() const
{
    QVariantMap ret;

    ret.insert("type-id", type());
    ret.insert("name", typeid(*this).name());
    ret.insert("parameters", parameter());

    return ret;
}

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
    : ptr(other.ptr ? other.ptr->clone() : nullptr)
{
}

inline DataModelPtr& DataModelPtr::operator=(const DataModelPtr &other)
{
    ptr.reset(other.ptr ? other.ptr->clone() : nullptr);
    return *this;
}



} // namespace CTL

#endif // ABSTRACTDATAMODEL_H
