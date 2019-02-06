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

class CTsystem : public SerializationInterface
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

    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

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

private:
    QString _name; //!< The name of the system.
    std::vector<ComponentPtr> _componentList; //!< The list of components.
};

} // namespace CTL

/*! \file */

#endif // CTSYSTEM_H
