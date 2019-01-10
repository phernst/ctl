#include "ctsystembuilder.h"
#include "ctsystem.h"

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

CTsystem CTsystemBuilder::createFromJSON(const QJsonObject &json)
{
    auto name = json.value("name").toString();

    CTsystem ret(name);

    auto jsonArray = json.value("components").toArray();
    for(auto comp : jsonArray)
        ret.addComponent(makeComponentFromJson(comp.toObject()));

    return ret;
}

}
