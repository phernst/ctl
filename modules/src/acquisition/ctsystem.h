#ifndef CTL_CTSYSTEM_H
#define CTL_CTSYSTEM_H

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
 * \class CTSystem
 *
 * \brief The CTSystem class manages the list of all components of a system.
 *
 * This class is used to manage the list of system components. System components can be added
 * using addComponent() or the "stream" operator<<(). The full component list can by filtered by
 * the components' base types in order to assess, for example all detector components in the system.
 *
 * \code
 *   CTSystem mySystem;
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

class CTSystem : public SerializationInterface
{
public:
    typedef std::unique_ptr<SystemComponent> ComponentPtr; //!< Alias for unique pointer to SystemComponent

    CTSystem(QString name = defaultName());
    CTSystem(const CTSystem& other);
    CTSystem(CTSystem&& other) = default;
    CTSystem& operator=(CTSystem&& other) = default;
    CTSystem& operator=(const CTSystem& other);

    virtual ~CTSystem() override = default;

    // virtual methods
    virtual CTSystem* clone() const &;
    virtual CTSystem* clone() &&;
    virtual QString info() const;
    virtual QString overview() const;

    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter methods
    std::vector<AbstractDetector*> detectors() const;
    std::vector<AbstractGantry*> gantries() const;
    std::vector<AbstractSource*> sources() const;
    std::vector<AbstractBeamModifier*> modifiers() const;

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
    CTSystem& operator<<(ComponentPtr component);
    CTSystem& operator<<(SystemComponent* component);

private:
    QString _name; //!< The name of the system.
    std::vector<ComponentPtr> _componentList; //!< The list of components.
};

Q_DECL_DEPRECATED_X("Class has been renamed. Please consider the new spelling 'CTSystem'.")
typedef CTSystem CTsystem;

} // namespace CTL

/*! \file */

#endif // CTL_CTSYSTEM_H
