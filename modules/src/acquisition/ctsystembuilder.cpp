#include "ctsystembuilder.h"
#include "ctsystem.h"
#include "io/jsonserializer.h"

namespace CTL {

/*!
 * Constructs a CTSystem based on the definitions in \a systemBlueprint
 */
CTSystem CTSystemBuilder::createFromBlueprint(const AbstractCTSystemBlueprint& systemBlueprint)
{
    CTSystem ret(systemBlueprint.systemName());

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
 * Constructs a CTSystem from the information in the JSON file \a fileName.
 *
 * Uses JsonSerializer to deserialize data from the file.
 */
CTSystem CTSystemBuilder::createFromJSONFile(const QString& fileName)
{
    JsonSerializer serializer;
    auto deserializedSys = serializer.deserializeSystem(fileName);

    CTSystem ret(std::move(*deserializedSys));

    return ret;
}

/*!
 * \fn virtual AbstractDetector* AbstractCTSystemBlueprint::detector() const = 0
 *
 * Constructs the detector component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific detector component
 * that shall be used in the system.
 */

/*!
 * \fn virtual AbstractGantry* AbstractCTSystemBlueprint::gantry() const = 0
 *
 * Constructs the gantry component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific gantry component
 * that shall be used in the system.
 */

/*!
 * \fn virtual AbstractSource* AbstractCTSystemBlueprint::source() const = 0
 *
 * Constructs the source component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific source component
 * that shall be used in the system.
 */

/*!
 * \fn AbstractCTSystemBlueprint::AbstractCTSystemBlueprint()
 *
 * Constructs the source component for the blueprinted system. The caller must take care of the
 * ownership of the constructed object.
 *
 * Implement this method in derived classes such that it constructs the specific source component
 * that shall be used in the system.
 */

/*!
 * \fn AbstractCTSystemBlueprint::AbstractCTSystemBlueprint()
 *
 * Standard default constructor.
 */

/*!
 * \fn AbstractCTSystemBlueprint::AbstractCTSystemBlueprint(const AbstractCTSystemBlueprint&)
 *
 * Standard copy constructor.
 */

/*!
 * \fn AbstractCTSystemBlueprint::AbstractCTSystemBlueprint(AbstractCTSystemBlueprint&&)
 *
 * Standard move constructor.
 */

/*!
 * \fn AbstractCTSystemBlueprint& AbstractCTSystemBlueprint::operator= (const AbstractCTSystemBlueprint&)
 *
 * Standard copy assignment operator.
 */

/*!
 * \fn AbstractCTSystemBlueprint& AbstractCTSystemBlueprint::operator= (AbstractCTSystemBlueprint&&)
 *
 * Standard move assignment operator.
 */

/*!
 * \fn virtual AbstractCTSystemBlueprint::~AbstractCTSystemBlueprint()
 *
 * Default destructor. Virtual for purpose of polymorphism.
 */

/*!
 * \fn QString AbstractCTSystemBlueprint::systemName() const
 *
 * Returns the name of the blueprinted system.
 *
 * Default return value is "Blueprinted system". Reimplement this method in derived classes to
 * return the desired system name.
 */

/*!
 * \fn std::vector<GenericBeamModifier*> AbstractCTSystemBlueprint::modifiers() const
 *
 * Constructs all beam modifier components blueprinted for the system and returns a vector
 * containing pointers to all constructed objects.
 *
 * (Optionally) implement this method in derived classes such that it constructs the specific
 * beam modifier components that shall be used in the system.
 */

} // namespace CTL
