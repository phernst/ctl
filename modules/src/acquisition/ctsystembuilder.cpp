#include "ctsystembuilder.h"
#include "ctsystem.h"
#include "io/jsonserializer.h"

namespace CTL {

CTsystem CTsystemBuilder::createFromBlueprint(const AbstractCTsystemBlueprint &systemBlueprint)
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

CTsystem CTsystemBuilder::createFromJSONFile(const QString& fileName)
{
    JsonSerializer serializer;
    auto deserializedSys = serializer.deserializeSystem(fileName);

    CTsystem system(*std::move(deserializedSys));
    delete deserializedSys;

    return system;
}

}
