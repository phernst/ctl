#include "abstractdatamodel.h"
#include "datamodeloperations.h"

namespace CTL {

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

QVariant AbstractDataModel::parameter() const { return QVariant(); }

void AbstractDataModel::setParameter(const QVariant&) {}

void AbstractDataModel::setName(const QString &name) { _name = name; }

const QString& AbstractDataModel::name() const { return _name; }

// Use SerializationInterface::toVariant() documentation.
QVariant AbstractDataModel::toVariant() const
{
    QVariantMap ret = SerializationInterface::toVariant().toMap();

    QString nameString = _name.isEmpty() ? typeid(*this).name()
                                         : _name;
    ret.insert("name", nameString);
    ret.insert("parameters", parameter());

    return ret;
}

// Use SerializationInterface::fromVariant() documentation.
void AbstractDataModel::fromVariant(const QVariant& variant)
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

bool AbstractDataModel::isIntegrable() const
{
    return dynamic_cast<const AbstractIntegrableDataModel*>(this);
}

DataModelPtr::DataModelPtr(std::unique_ptr<AbstractDataModel> model)
    : ptr(std::move(model))
{
}

DataModelPtr::DataModelPtr(const DataModelPtr& other)
    : ptr(other.ptr ? other->clone() : nullptr)
{
}

DataModelPtr& DataModelPtr::operator=(const DataModelPtr &other)
{
    ptr.reset(other.ptr ? other->clone() : nullptr);
        return *this;
}

AbstractDataModel* DataModelPtr::operator->() const
{
    Q_ASSERT(ptr);
    return ptr.get();
}

AbstractDataModel& DataModelPtr::operator*() const
{
    Q_ASSERT(ptr);
    return *ptr;
}

AbstractDataModel* DataModelPtr::get() const
{
    return ptr.get();
}

void DataModelPtr::reset(AbstractDataModel* model)
{
    ptr.reset(model);
}

} // namespace CTL
