#include "systemcomponent.h"
#include "jsonparser.h"

namespace CTL {

/*!
 * Constructs a component with the name \a name. If no name is passed, it defaults to
 * "Generic system component".
 */
SystemComponent::SystemComponent(const QString& name)
    : _name(name)
{
}

/*!
 * Constructs a SystemComponent from a QJsonObject.
 */
SystemComponent::SystemComponent(const QJsonObject& json) { SystemComponent::read(json); }

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

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void SystemComponent::read(const QJsonObject& json) { _name = json.value("name").toString(); }

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
void SystemComponent::write(QJsonObject& json) const
{
    json.insert("type-id", this->type());
    json.insert("generic type-id", this->elementalType());
    json.insert("name", _name);
}

std::unique_ptr<SystemComponent> makeComponentFromJson(const QJsonObject& object,
                                                       bool fallbackToGenericType)
{
    std::unique_ptr<SystemComponent> ret(parseComponentFromJson(object));
    // unknown type
    if((ret == nullptr) && fallbackToGenericType)
        ret.reset(parseGenericComponentFromJson(object));
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
