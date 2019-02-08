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
    auto ptr = parse(variant, instance().componentFactories());
    auto castedPtr = dynamic_cast<SystemComponent*>(ptr);
    if(castedPtr == nullptr) // conversion not successful
        delete ptr;
    return castedPtr;
}

AbstractDataModel* SerializationHelper::parseDataModel(const QVariant& variant)
{
    auto ptr = parse(variant, instance().modelFactories());
    auto castedPtr = dynamic_cast<AbstractDataModel*>(ptr);
    if(castedPtr == nullptr) // conversion not successful
        delete ptr;
    return castedPtr;
}

AbstractPrepareStep* SerializationHelper::parsePrepareStep(const QVariant& variant)
{
    auto ptr = parse(variant, instance().prepareStepFactories());
    auto castedPtr = dynamic_cast<AbstractPrepareStep*>(ptr);
    if(castedPtr == nullptr) // conversion not successful
        delete ptr;
    return castedPtr;
}

SerializationInterface* SerializationHelper::parseMiscObject(const QVariant& variant)
{
    return parse(variant, instance().miscFactories());
}

SerializationInterface* SerializationHelper::parse(const QVariant& variant,
                                                   const FactoryMap& factoryMap)
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    if(!factoryMap.contains(typeID))
        return nullptr;
    return factoryMap[typeID](variant);
}

} // namespace CTL
