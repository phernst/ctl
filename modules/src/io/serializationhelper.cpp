#include "serializationhelper.h"

#include "acquisition/abstractpreparestep.h"
#include "components/systemcomponent.h"
#include "models/abstractdatamodel.h"

namespace CTL {

SerializationHelper& SerializationHelper::instance()
{
    static SerializationHelper helper;
    return helper;
}

const QMap<int, SerializationHelper::SerializableFactoryFunction>&
SerializationHelper::componentFactories() const
{
    return _componentFactories;
}

const QMap<int, SerializationHelper::SerializableFactoryFunction>&
SerializationHelper::modelFactories() const
{
    return _modelFactories;
}

const QMap<int, SerializationHelper::SerializableFactoryFunction>&
SerializationHelper::prepareStepFactories() const
{
    return _prepareStepFactories;
}

const QMap<int, SerializationHelper::SerializableFactoryFunction>&
SerializationHelper::miscFactories() const
{
    return _miscFactories;
}

SystemComponent* SerializationHelper::parseComponent(const QVariant& variant)
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    const auto& factoryMap = instance().componentFactories();
    if(!factoryMap.contains(typeID))
        return nullptr;
    return dynamic_cast<SystemComponent*>(factoryMap[typeID](variant));
}

AbstractDataModel* SerializationHelper::parseDataModel(const QVariant& variant)
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    const auto& factoryMap = instance().modelFactories();
    if(!factoryMap.contains(typeID))
        return nullptr;
    return dynamic_cast<AbstractDataModel*>(factoryMap[typeID](variant));
}

AbstractPrepareStep* SerializationHelper::parsePrepareStep(const QVariant& variant)
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    const auto& factoryMap = instance().prepareStepFactories();
    if(!factoryMap.contains(typeID))
        return nullptr;
    return dynamic_cast<AbstractPrepareStep*>(factoryMap[typeID](variant));
}

SerializationInterface* SerializationHelper::parseSerializableObject(const QVariant& variant)
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    const auto& factoryMap = instance().miscFactories();
    if(!factoryMap.contains(typeID))
        return nullptr;
    return factoryMap[typeID](variant);
}

} // namespace CTL
