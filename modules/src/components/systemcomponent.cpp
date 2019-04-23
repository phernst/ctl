#include "systemcomponent.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(SystemComponent)

/*!
 * Constructs a component with the name \a name. If no name is passed, it defaults to
 * "Generic system component".
 */
SystemComponent::SystemComponent(const QString& name)
    : _name(name)
{
}

/*!
 * Returns the default name for the component: "Generic system component".
 */
QString SystemComponent::defaultName()
{
    static const QString defName(QStringLiteral("Generic system component"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * Returns a string that should contain all information about the component.
 */
QString SystemComponent::info() const
{
    return "Object(" + QString(typeid(*this).name()) + ") {\n\tName: " + _name + "\n";
}

/*!
 * Constructs a copy of the object and returns a base class pointer to it.
 */
SystemComponent* SystemComponent::clone() const { return new SystemComponent(*this); }

/*!
 * Returns the objects name.
 */
const QString& SystemComponent::name() const { return _name; }

/*!
 * Sets the objects name to \a name.
 */
void SystemComponent::rename(QString name) { _name = std::move(name); }

/*!
 * Returns a string that contains the typename of \a type.
 */
QString SystemComponent::typeInfoString(const std::type_info& type)
{
    return QString(" -------" + QString(type.name())).leftJustified(56, '-') + "\n";
}

// Use SerializationInterface::fromVariant() documentation.
void SystemComponent::fromVariant(const QVariant& variant)
{
    _name = variant.toMap().value("name").toString();
}

// Use SerializationInterface::toVariant() documentation.
QVariant SystemComponent::toVariant()const
{
    QVariantMap ret;
    ret.insert("type-id", this->type());
    ret.insert("generic type-id", this->elementalType());
    ret.insert("name", _name);

    return ret;
}

/*!
 * \fn virtual SystemComponent::~SystemComponent()
 *
 * Default destructor.
 */

/*!
 * \fn SystemComponent& SystemComponent::operator=(SystemComponent&&)
 *
 * Default move-assignment operator.
 */

/*!
 * \fn SystemComponent::SystemComponent(SystemComponent&&)
 *
 * Default move-constructor.
 */

/*!
 * \fn SystemComponent::SystemComponent(const SystemComponent&)
 *
 * Default copy-constructor.
 */

/*!
 * \fn SystemComponent& SystemComponent::operator=(const SystemComponent&)
 *
 * Default assignment operator.
 */


} // namespace CTL
