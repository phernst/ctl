#include "ctsystem.h"
#include "components/allgenerictypes.h"

namespace CTL {

/*!
 * \fn CTsystem::CTsystem(CTsystem&& other)
 * Default move constructor.
 */

/*!
 * \fn CTsystem& CTsystem::operator=(CTsystem&& other) =
 * Default move assignment operator.
 */

/*!
 * \fn CTsystem::~CTsystem()
 * Default destructor.
 */

/*!
 * Constructs a CTsystem object named \a name.
 */
CTsystem::CTsystem(const QString& name)
    : _name(name)
{
}

/*!
 * Constructs a copy of \a other.
 */
CTsystem::CTsystem(const CTsystem& other)
    : _name(other.name())
{
    _componentList.reserve(other.components().size());
    for(const auto& otherComp : other.components())
        _componentList.emplace_back(otherComp->clone());
}

/*!
 * Assigns the content of \a other to this system and returns a reference to this instance.
 */
CTsystem& CTsystem::operator=(const CTsystem& other)
{
    if(this != &other)
    {
        _name = other.name();
        _componentList.resize(other.nbComponents());
        auto compIt = _componentList.begin();
        for(const auto& otherComp : other.components())
            compIt++->reset(otherComp->clone());
    }
    return *this;
}

/*!
 * Returns a string that contains full information about all components in the system.
 */
QString CTsystem::info() const
{
    QString ret("CT system: ");
    ret.append(_name + " {\n");
    for(const auto& comp : _componentList)
        ret.append(comp->info());
    ret.append("}\n");

    return ret;
}

/*!
 * Returns a string that gives an overview over this system. This contains the system's
 * name and the number of components as well as their names. Additionally, is shows whether
 * the system is valid and simple or not.
 *
 * \sa isValid(), is Simple().
 */
QString CTsystem::overview() const
{
    QString ret("CT system: ");
    ret += _name
        + "\n"
          "\tNumber of components: "
        + QString::number(_componentList.size()) + "\n";
    ret += isValid() ? "\tSystem is valid: true\n" : "\tSystem is valid: false\n";
    ret += isSimple() ? "\tSystem is simple: true\n" : "\tSystem is simple: false\n";
    ret += "----------------------------------\nComponents:\n";
    for(const auto& comp : _componentList)
        ret.append("\t(*) " + comp->name() + "\n");

    return ret;
}

/*!
 * Returns true if the number of compontents is zero, otherwise it returns false.
 */
bool CTsystem::isEmpty() const
{
    return this->nbComponents() == 0;
}

/*!
 * Returns true if the system is valid. To be valid, a system must have <b>at least one</b>
 * component of each of the following base types (or their derived classes):
 * \li AbstractGantry
 * \li AbstractDetector
 * \li AbstractSource.
 *
 * The system may have an arbitrary number of AbstractBeamModifier components and still be valid.
 */
bool CTsystem::isValid() const
{
    bool hasDetector = !detectors().isEmpty();
    bool hasGantry = !gantries().isEmpty();
    bool hasSource = !sources().isEmpty();

    return hasDetector && hasGantry && hasSource;
}

/*!
 * Returns true if the system is simple. To be simple, a system must have <b>exactly one</b>
 * component of each of the following base types (or their derived classes):
 * \li AbstractGantry
 * \li AbstractDetector
 * \li AbstractSource.
 *
 * The system may have an arbitrary number of AbstractBeamModifier components ans still be simple.
 */
bool CTsystem::isSimple() const
{
    return (detectors().length() == 1) && (gantries().length() == 1) && (sources().length() == 1);
}

/*!
 * Returns the default name for the component: "Generic CT-system".
 */
QString CTsystem::defaultName()
{
    static const QString defName(QStringLiteral("Generic CT-system"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * Returns a list of all components of elementary type AbstractDetector in the system.
 */
QList<AbstractDetector*> CTsystem::detectors() const
{
    QList<AbstractDetector*> ret;

    for(const auto& comp : _componentList)
        if(comp->elementalType() == AbstractDetector::Type)
            ret.append(static_cast<AbstractDetector*>(comp.get()));

    return ret;
}

/*!
 * Returns a list of all components of elementary type AbstractGantry in the system.
 */
QList<AbstractGantry*> CTsystem::gantries() const
{
    QList<AbstractGantry*> ret;

    for(const auto& comp : _componentList)
        if(comp->elementalType() == AbstractGantry::Type)
            ret.append(static_cast<AbstractGantry*>(comp.get()));

    return ret;
}

/*!
 * Returns a list of all components of elementary type AbstractSource in the system.
 */
QList<AbstractSource *> CTsystem::sources() const
{
    QList<AbstractSource*> ret;

    for(const auto& comp : _componentList)
        if(comp->elementalType() == AbstractSource::Type)
            ret.append(static_cast<AbstractSource*>(comp.get()));

    return ret;
}

/*!
 * Returns a list of all components of elementary type AbstractBeamModifier in the system.
 */
QList<AbstractBeamModifier*> CTsystem::modifiers() const
{
    QList<AbstractBeamModifier*> ret;

    for(const auto& comp : _componentList)
        if(comp->elementalType() == AbstractBeamModifier::Type)
            ret.append(static_cast<AbstractBeamModifier*>(comp.get()));

    return ret;
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void CTsystem::read(const QJsonObject &json)
{
    _name = json.value("name").toString();
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
void CTsystem::write(QJsonObject &json) const
{
    json.insert("name", _name);

    QJsonArray componentArray;

    for(const auto& comp : _componentList)
    {
        QJsonObject tmp;
        comp->write(tmp);
        componentArray.append(tmp);
    }

    json.insert("components", componentArray);
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void CTsystem::fromVariant(const QVariant &variant)
{
    QVariantMap varMap = variant.toMap();

    this->rename(varMap.value("name").toString());

    // fill in components
    this->clear();
    QVariantList componentVariantList = varMap.value("components").toList();
    for(const auto& comp : componentVariantList)
        this->addComponent(SerializationHelper::parseComponent(comp));
}

/*!
 * Writes all components to a QVariant. This uses SerializationInterface::toVariant() of individual
 * components in the system.
 */
QVariant CTsystem::toVariant() const
{
    QVariantMap ret;

    ret.insert("name", _name);

    QVariantList componentVariantList;

    for(const auto& comp : _componentList)
        componentVariantList.append(comp->toVariant());

    ret.insert("components", componentVariantList);

    return ret;
}

/*!
 * Constructs a copy of the system and returns a base-class pointer to the new object.
 */
CTsystem* CTsystem::clone() const & { return new CTsystem(*this); }

/*!
 * Constructs a new CTsystem object and moves the content of this instance into the new
 * object. Returns a base class pointer to the new instance.
 */
CTsystem* CTsystem::clone() && { return new CTsystem(std::move(*this)); }

/*!
 * Returns a constant reference to the list of components.
 */
const std::vector<CTsystem::ComponentPtr>& CTsystem::components() const { return _componentList; }

/*!
 * Returns the name of the system.
 */
const QString& CTsystem::name() const { return _name; }

/*!
 * Returns the number of components in the system.
 *
 * Same as components().size().
 */
uint CTsystem::nbComponents() const { return static_cast<uint>(_componentList.size()); }

/*!
 * Adds \a component to the system. Does nothing if \a component is \c null.
 */
void CTsystem::addComponent(CTsystem::ComponentPtr component)
{
    if(component)
        _componentList.push_back(std::move(component));
}

/*!
 * Adds \a component to the system. This CTsystem instance takes ownership of
 * \a component.
 *
 * Does nothing if \a component is \c null.
 */
void CTsystem::addComponent(SystemComponent* component)
{
    if(component)
        _componentList.push_back(ComponentPtr(component));
}

/*!
 * Removes all components from the system.
 */
void CTsystem::clear()
{
    _componentList.clear();
}

/*!
 * Sets the system's name to \a name.
 */
void CTsystem::rename(QString name) { _name = std::move(name); }

/*!
 * Removes the component \a component from the system.
 */
void CTsystem::removeComponent(SystemComponent *component)
{
    std::vector<ComponentPtr> newComponentList;
    newComponentList.reserve(nbComponents());

    // move all components except the component to be removed (i.e. 'component')
    // to 'newComponentList'
    for(auto& comp : _componentList)
        if(comp.get() != component)
            newComponentList.push_back(std::move(comp));

    // swap-in 'newComponentList'
    _componentList = std::move(newComponentList);
}

/*!
 * Operator style alternative to add \a component to the system.
 *
 * Similar to addComponent() but also returns a reference to this instance.
 */
CTsystem& CTsystem::operator<<(CTsystem::ComponentPtr component)
{
    addComponent(std::move(component));
    return *this;
}

/*!
 * Operator style alternative to add \a component to the system. This CTsystem instance takes
 * ownership of \a component.
 *
 * Similar to addComponent() but also returns a reference to this instance.
 */
CTsystem& CTsystem::operator<<(SystemComponent* component)
{
    addComponent(component);
    return *this;
}

} // namespace CTL
