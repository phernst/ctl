#ifndef SIMPLESYSTEM_H
#define SIMPLESYSTEM_H

#include "ctsystem.h"

namespace CTL {

/*!
 * \class SimpleCTsystem
 *
 * \brief The SimpleCTsystem class is a specialized sub-class of CTsystem for simple systems (i.e.
 * with a single gantry, detector and source).
 *
 * This class is a specialization of CTsystem to describe systems that have the simple configuration
 * that involves only a single source and detector mounted on a single gantry component.
 *
 * The CTL framework is based on geometry description that uses projection matrices. These can only
 * fully describe the system geometry for this type of system configuration. Consequently, all
 * further parts of the framework (e.g. geometry encoding, definition of acquisition setups) rely
 * on the SimpleCTsystem class (and their sub-classes).
 *
 * Instances of this class can be created from "regular" CTsystem objects using the factory method
 * fromCTsystem(). This assures that the configuration of the system is simple. Alternatively, a
 * constructor taking all three essential components can also be used. As it is required
 * for SimpleCTsystem objects at all times to retain a simple configuration, adding further
 * components using addComponent() (or removing components with removeComponent()) is no longer
 * allowed. Instead, beam modifiers - these can be contained in an arbitrary number - can be added
 * using addBeamModifier(). For convenience, each of the three essential components can, however, be
 * replaced by another component of the appropriate type using the corresponding methods
 * replaceDetector(), replaceGantry() and replaceSource().
 *
 *  \code
 *    CTsystem mySystem("A simple system");
 *
 *    // We now create some components and add them to the system.
 *    AbstractDetector* myDetector = new FlatPanelDetector(QSize(100, 100), QSizeF(1.0f, 1.0f));
 *    AbstractSource* mySource = new XrayTube(120.0, 1.0);
 *    AbstractGantry* myGantry = new TubularGantry(1000.0, 500.0);
 *
 *    mySystem << myDetector << mySource;
 *
 *    // The system is still missing a gantry component by now.
 *    SimpleCTsystem simpleSystem = SimpleCTsystem::fromCTsystem(mySystem);
 *    std::cout << "simpleSystem is valid: " << simpleSystem.isValid() << std::endl;
 *    // Output: simpleSystem is valid: 0
 *
 *    // We now add the missing gantry.
 *    mySystem << myGantry;
 *    simpleSystem = SimpleCTsystem::fromCTsystem(mySystem);
 *    std::cout << "simpleSystem is valid: " << simpleSystem.isValid() << std::endl;
 *    // Output: simpleSystem is valid: 1
 *
 *    // We can now easily access the central components:
 *    AbstractDetector* theDetector = simpleSystem.detector();
 *    std::cout << theDetector->info().toStdString();
 *    // *** Output ***
 *    // Object(N3CTL17FlatPanelDetectorE) {
 *    //         Name: Flat panel detector (2)
 *    //  -------PKN3CTL16AbstractDetectorE----------------------
 *    //         Nb. of pixels per module: 100 x 100
 *    //         Pixel dimensions: 1 mm x 1 mm
 *    //  -------PKN3CTL17FlatPanelDetectorE---------------------
 *    // }
 *
 *    // Other example: change the rotation angle of the tubular gantry
 *    TubularGantry* theGantry = dynamic_cast<TubularGantry*>(simpleSystem.gantry());
 *    theGantry->setRotationAngle(90.0_deg);
 *    std::cout << theGantry->info().toStdString();
 *    // *** Output ***
 *    // Object(N3CTL13TubularGantryE) {
 *    //         Name: Tubular gantry (2)
 *    //  -------PKN3CTL14AbstractGantryE------------------------
 *    //         Source Displacement: (0 mm, 0 mm, 0 mm)
 *    //         -Rotation:
 *    //         |1,000000___0,000000___0,000000|
 *    //         |0,000000___1,000000___0,000000|
 *    //         |0,000000___0,000000___1,000000|
 *    //         Detector Displacement: (0 mm, 0 mm, 0 mm)
 *    //         -Rotation:
 *    //         |1,000000___0,000000___0,000000|
 *    //         |0,000000___1,000000___0,000000|
 *    //         |0,000000___0,000000___1,000000|
 *    //  -------PKN3CTL13TubularGantryE-------------------------
 *    //         Source-to-detector distance: 1000 mm
 *    //         Source-to-iso-center distance: 500 mm
 *    //         Rotation angle: 90 deg
 *    //         Table pitch position: 0 mm
 *    //         Tilt angle: 0 deg
 *    // }
 *
 * \endcode
 *
 * \sa isSimple().
 */
class SimpleCTsystem : public CTsystem
{
public:
    // ctor
    SimpleCTsystem(const AbstractDetector& detector,
                   const AbstractGantry& gantry,
                   const AbstractSource& source);
    SimpleCTsystem(AbstractDetector&& detector,
                   AbstractGantry&& gantry,
                   AbstractSource&& source);

    // factory ctor
    static SimpleCTsystem fromCTsystem(const CTsystem& system, bool* ok = nullptr);
    static SimpleCTsystem fromCTsystem(CTsystem&& system, bool* ok = nullptr);

    // virtual methods
    CTsystem* clone() const & override;
    CTsystem* clone() && override;

    // getter methods
    AbstractDetector* detector() const;
    AbstractGantry* gantry() const;
    AbstractSource* source() const;

    // replacer
    void replaceDetector(AbstractDetector* newDetector);
    void replaceGantry(AbstractGantry* newGantry);
    void replaceSource(AbstractSource* newSource);

    // other
    void addBeamModifier(AbstractBeamModifier* modifier);
    void addBeamModifier(std::unique_ptr<AbstractBeamModifier> modifier);

protected:
    SimpleCTsystem() = default;
    SimpleCTsystem(const CTsystem&);
    SimpleCTsystem(CTsystem&& system);

private:
    using CTsystem::addComponent;
    using CTsystem::clear;
    using CTsystem::removeComponent;
    using CTsystem::operator<<;
};

// use documentation of CTsystem::clone()
inline CTsystem* SimpleCTsystem::clone() const & { return new SimpleCTsystem(*this); }

// use documentation of CTsystem::clone()
inline CTsystem* SimpleCTsystem::clone() && { return new SimpleCTsystem(std::move(*this)); }

} // namespace CTL

/*! \file */

#endif // SIMPLESYSTEM_H
