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

} // namespace CTL
