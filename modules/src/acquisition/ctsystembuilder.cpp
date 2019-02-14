#include "ctsystembuilder.h"
#include "ctsystem.h"
#include "io/jsonserializer.h"

namespace CTL {

/*!
 * Constructs a CTsystem based on the definitions in \a systemBlueprint
 */
CTsystem CTsystemBuilder::createFromBlueprint(const AbstractCTsystemBlueprint& systemBlueprint)
{
    CTsystem ret(systemBlueprint.systemName());

    // add basic components
    ret.addComponent(systemBlueprint.detector());
    ret.addComponent(systemBlueprint.gantry());
    ret.addComponent(systemBlueprint.source());

    // add (optional) modifiers
    auto modifiers = systemBlueprint.modifiers();
    for(auto modifier : modifiers)
        ret.addComponent(modifier);

    return ret;
}

/*!
 * Constructs a CTsystem from the information in the JSON file \a fileName.
 *
 * Uses JsonSerializer to deserialize data from the file.
 */
CTsystem CTsystemBuilder::createFromJSONFile(const QString& fileName)
{
    JsonSerializer serializer;
    auto deserializedSys = serializer.deserializeSystem(fileName);

    CTsystem ret(std::move(*deserializedSys));

    return ret;
}

/*!
 * \fn virtual AbstractDetector* AbstractCTsystemBlueprint::detector() const = 0
 *
 * Constructs the detector component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific detector component
 * that shall be used in the system.
 */

/*!
 * \fn virtual AbstractGantry* AbstractCTsystemBlueprint::gantry() const = 0
 *
 * Constructs the gantry component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific gantry component
 * that shall be used in the system.
 */

/*!
 * \fn virtual AbstractSource* AbstractCTsystemBlueprint::source() const = 0
 *
 * Constructs the source component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific source component
 * that shall be used in the system.
 */

/*!
 * \fn AbstractCTsystemBlueprint::AbstractCTsystemBlueprint()
 *
 * Constructs the source component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific source component
 * that shall be used in the system.
 */

/*!
 * \fn AbstractCTsystemBlueprint::AbstractCTsystemBlueprint()
 *
 * Standard default constructor.
 */

/*!
 * \fn AbstractCTsystemBlueprint::AbstractCTsystemBlueprint(const AbstractCTsystemBlueprint&)
 *
 * Standard copy constructor.
 */

/*!
 * \fn AbstractCTsystemBlueprint::AbstractCTsystemBlueprint(AbstractCTsystemBlueprint&&)
 *
 * Standard move constructor.
 */

/*!
 * \fn AbstractCTsystemBlueprint& AbstractCTsystemBlueprint::operator= (const AbstractCTsystemBlueprint&)
 *
 * Standard copy assignment operator.
 */

/*!
 * \fn AbstractCTsystemBlueprint& AbstractCTsystemBlueprint::operator= (AbstractCTsystemBlueprint&&)
 *
 * Standard move assignment operator.
 */

/*!
 * \fn virtual AbstractCTsystemBlueprint::~AbstractCTsystemBlueprint()
 *
 * Default destructor. Virtual for purpose of polymorphism.
 */

/*!
 * \fn QString AbstractCTsystemBlueprint::systemName() const
 *
 * Returns the name of the blueprinted system.
 *
 * Default return value is "Blueprinted system". Reimplement this method in derived classes to
 * return the desired system name.
 */

/*!
 * \fn std::vector<GenericBeamModifier*> AbstractCTsystemBlueprint::modifiers() const
 *
 * Constructs all beam modifier components blueprinted for the system and returns a vector
 * containing pointers to all constructed objects.
 *
 * (Optionally) implement this method in derived classes such that it constructs the specific
 * beam modifier components that shall be used in the system.
 */

} // namespace CTL
