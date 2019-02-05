#include "jsonserializer.h"
#include "components/systemcomponent.h"
#include "models/abstractdatamodel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace CTL {

JsonSerializer& JsonSerializer::instance()
{
    static JsonSerializer jsonModelParser;
    return jsonModelParser;
}

QMap<int, JsonSerializer::SerializableFactoryFunction>& JsonSerializer::componentFactories()
{
    return _componentFactories;
}

QMap<int, JsonSerializer::SerializableFactoryFunction>& JsonSerializer::modelFactories()
{
    return _modelFactories;
}

QMap<int, JsonSerializer::SerializableFactoryFunction>& JsonSerializer::prepareStepFactories()
{
    return _prepareStepFactories;
}

void JsonSerializer::serialize(const AbstractDataModel &model, const QString &fileName)
{
    serialize(static_cast<const SerializationInterface&>(model), fileName);
}

void JsonSerializer::serialize(const AbstractPrepareStep &prepStep, const QString &fileName)
{
    // serialize(static_cast<const SerializationInterface&>(prepStep), fileName);
}

void JsonSerializer::serialize(const SystemComponent &component, const QString &fileName)
{
    serialize(static_cast<const SerializationInterface&>(component), fileName);
}

void JsonSerializer::serialize(const SerializationInterface &serializableObject,
                               const QString& fileName)
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::WriteOnly);
    QJsonDocument doc(convertVariantToJsonObject(serializableObject.toVariant()));
    saveFile.write(doc.toJson());
    saveFile.close();
}

SystemComponent* JsonSerializer::deserializeComponent(const QString &fileName)
{
    return parseComponent(variantFromJsonFile(fileName));
}

AbstractDataModel* JsonSerializer::deserializeDataModel(const QString &fileName)
{
    return parseDataModel(variantFromJsonFile(fileName));
}

AbstractPrepareStep* JsonSerializer::deserializePrepareStep(const QString &fileName)
{
    return parsePrepareStep(variantFromJsonFile(fileName));
}

QVariant JsonSerializer::variantFromJsonFile(const QString &fileName)
{
    QFile loadFile(fileName);
    loadFile.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
    loadFile.close();

    return doc.toVariant();
}

SystemComponent* JsonSerializer::parseComponent(const QVariant &variant)
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

AbstractDataModel* JsonSerializer::parseDataModel(const QVariant &variant)
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

AbstractPrepareStep* JsonSerializer::parsePrepareStep(const QVariant &variant)
{
    /* TO BE DONE */

    return nullptr;
}

QJsonObject JsonSerializer::convertVariantToJsonObject(const QVariant &variant)
{
    QJsonObject ret;

    if(variant.canConvert(QMetaType::QVariantMap))
        ret = QJsonObject::fromVariantMap(variant.toMap());
    else
        qWarning() << "JsonSerializer::serialize: Cannot serialize variant. Incompatible "
                      "information.";

    return ret;
}

void SerializationInterface::fromVariant(const QVariant&)
{
}

QVariant SerializationInterface::toVariant() const
{
    return QVariant();
}

} // namespace CTL
