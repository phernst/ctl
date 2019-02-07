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
    return dynamic_cast<SystemComponent*>(parse(variant, instance().componentFactories()));
}

AbstractDataModel* SerializationHelper::parseDataModel(const QVariant& variant)
{
    return dynamic_cast<AbstractDataModel*>(parse(variant, instance().modelFactories()));
}

AbstractPrepareStep* SerializationHelper::parsePrepareStep(const QVariant& variant)
{
    return dynamic_cast<AbstractPrepareStep*>(parse(variant, instance().prepareStepFactories()));
}

SerializationInterface* SerializationHelper::parseSerializableObject(const QVariant& variant)
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
