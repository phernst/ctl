#ifndef CTSYSTEM_H
#define CTSYSTEM_H

#include "components/systemcomponent.h"

#include <QJsonArray>
#include <vector>

namespace CTL {

// forward declarations
class AbstractDetector;
class AbstractGantry;
class AbstractSource;
class AbstractBeamModifier;

/*!
 * \class CTsystem
 *
 * \brief The CTsystem class manages the list of all components of a system.
 *
 * This class is used to manage the list of system components. System components can be added
 * using addComponent() or the "stream" operator<<(). The full component list can by filtered by
 * the components' base types in order to assess, for example all detector components in the system.
 *
 * \code
 *   CTsystem mySystem;
 *   std::cout << mySystem.overview().toStdString();
 *   // *** Output ***
 *   // CT system: Generic CT-system
 *   //         Number of components: 0
 *   //         System is valid: false
 *   //         System is simple: false
 *   // ----------------------------------
 *   // Components:
 *
 *   mySystem.rename("Examplary system");
 *
 *   // We now create some components and add them to the system.
 *   AbstractDetector* myDetector = new FlatPanelDetector(QSize(100, 100), QSizeF(1.0f, 1.0f));
 *   AbstractSource* mySource = new XrayTube(120.0, 1.0);
 *   AbstractGantry* myGantry = new TubularGantry(1000.0, 500.0);
 *
 *   mySystem.addComponent(myDetector);
 *   mySystem.addComponent(mySource);
 *   mySystem.addComponent(myGantry);
 *
 *   std::cout << mySystem.overview().toStdString();
 *   // *** Output ***
 *   // CT system: Examplary system
 *   //         Number of components: 3
 *   //         System is valid: true
 *   //         System is simple: true
 *   // ----------------------------------
 *   // Components:
 *   //         (*) Flat panel detector
 *   //         (*) Xray tube
 *   //         (*) Tubular gantry
 *
 *   // Note that the system is valid and simple, as it contains exactly one source, detector and gantry.
 *
 *   // We now add another source to the system.
 *   AbstractSource* additionalSource = new XrayTube(70.0, 1.0, "Other tube");
 *
 *   mySystem.addComponent(additionalSource);
 *
 *   std::cout << mySystem.overview().toStdString();
 *   // *** Output ***
 *   // CT system: Examplary system
 *   //         Number of components: 4
 *   //         System is valid: true
 *   //         System is simple: false
 *   // ----------------------------------
 *   // Components:
 *   //         (*) Flat panel detector
 *   //         (*) Xray tube
 *   //         (*) Tubular gantry
 *   //         (*) Other tube
 *
 *   // Now the system has two sources. Hence it's still valid, but no longer simple.
 * \endcode
 */

class CTsystem
{
public:
    typedef std::unique_ptr<SystemComponent> ComponentPtr; //!< Alias for unique pointer to SystemComponent

    CTsystem(const QString& name = defaultName());
    CTsystem(const CTsystem& other);
    CTsystem(CTsystem&& other) = default;
    CTsystem& operator=(CTsystem&& other) = default;
    CTsystem& operator=(const CTsystem& other);

    virtual ~CTsystem() = default;

    // virtual methods
    virtual CTsystem* clone() const &;
    virtual CTsystem* clone() &&;
    virtual QString info() const;
    virtual QString overview() const;

    // getter methods
    QList<AbstractDetector*> detectors() const;
    QList<AbstractGantry*> gantries() const;
    QList<AbstractSource*> sources() const;
    QList<AbstractBeamModifier*> modifiers() const;

    const std::vector<ComponentPtr>& components() const;
    const QString& name() const;
    uint nbComponents() const;

    // other methods
    void addComponent(ComponentPtr component);
    void addComponent(SystemComponent* component);
    void clear();
    bool isEmpty() const;
    bool isValid() const;
    bool isSimple() const;
    void rename(QString name);
    void removeComponent(SystemComponent* component);

    // static methods
    static QString defaultName();

    // operators
    CTsystem& operator<<(ComponentPtr component);
    CTsystem& operator<<(SystemComponent* component);

    // JSON
    virtual void read(const QJsonObject& json);
    virtual void write(QJsonObject& json) const;

private:
    QString _name; //!< The name of the system.
    std::vector<ComponentPtr> _componentList; //!< The list of components.
};

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void CTsystem::read(const QJsonObject &json)
{
    _name = json.value("name").toString();
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
inline void CTsystem::write(QJsonObject &json) const
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
 * Constructs a copy of the system and returns a base-class pointer to the new object.
 */
inline CTsystem* CTsystem::clone() const & { return new CTsystem(*this); }

/*!
 * Constructs a new CTsystem object and moves the content of this instance into the new
 * object. Returns a base class pointer to the new instance.
 */
inline CTsystem* CTsystem::clone() && { return new CTsystem(std::move(*this)); }

/*!
 * Returns a constant reference to the list of components.
 */
inline const std::vector<CTsystem::ComponentPtr>& CTsystem::components() const { return _componentList; }

/*!
 * Returns the name of the system.
 */
inline const QString& CTsystem::name() const { return _name; }

/*!
 * Returns the number of components in the system.
 *
 * Same as components().size().
 */
inline uint CTsystem::nbComponents() const { return static_cast<uint>(_componentList.size()); }

/*!
 * Adds \a component to the system. Does nothing if \a component is \c null.
 */
inline void CTsystem::addComponent(CTsystem::ComponentPtr component)
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
inline void CTsystem::addComponent(SystemComponent* component)
{
    if(component)
        _componentList.push_back(ComponentPtr(component));
}

/*!
 * Removes all components from the system.
 */
inline void CTsystem::clear()
{
    _componentList.clear();
}

/*!
 * Sets the system's name to \a name.
 */
inline void CTsystem::rename(QString name) { _name = std::move(name); }

/*!
 * Removes the component \a component from the system.
 */
inline void CTsystem::removeComponent(SystemComponent *component)
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
inline CTsystem& CTsystem::operator<<(CTsystem::ComponentPtr component)
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
inline CTsystem& CTsystem::operator<<(SystemComponent* component)
{
    addComponent(component);
    return *this;
}

} // namespace CTL

/*! \file */

#endif // CTSYSTEM_H
