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

QMap<int, JsonSerializer::ComponentFactoryFunction>& JsonSerializer::componentFactories()
{
    return _componentFactories;
}

QMap<int, JsonSerializer::ModelFactoryFunction>& JsonSerializer::modelFactories()
{
    return _modelFactories;
}

QMap<int, JsonSerializer::PrepareStepFactoryFunction>& JsonSerializer::prepareStepFactories()
{
    return _prepareStepFactories;
}

void JsonSerializer::serializeComponent(const SystemComponent& component, const QString &fileName) const
{
    serializeVariant(component.toVariant(), fileName);
}

void JsonSerializer::serializeDataModel(const AbstractDataModel& model, const QString &fileName) const
{
    serializeVariant(model.toVariant(), fileName);
}

void JsonSerializer::serializePrepareStep(const AbstractPrepareStep &prepStep, const QString &fileName) const
{
    /* TO BE DONE */
}

SystemComponent* JsonSerializer::deserializeComponent(const QString &fileName) const
{
    return parseComponent(variantFromJsonFile(fileName));
}

AbstractDataModel* JsonSerializer::deserializeDataModel(const QString &fileName) const
{
    return parseDataModel(variantFromJsonFile(fileName));
}

AbstractPrepareStep* JsonSerializer::deserializePrepareStep(const QString &fileName) const
{
    return parsePrepareStep(variantFromJsonFile(fileName));
}

void JsonSerializer::serializeVariant(const QVariant& variant, const QString& fileName) const
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::WriteOnly);
    QJsonDocument doc(convertVariantToJsonObject(variant));
    saveFile.write(doc.toJson());
    saveFile.close();
}

QVariant JsonSerializer::variantFromJsonFile(const QString &fileName) const
{
    QFile loadFile(fileName);
    loadFile.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
    loadFile.close();

    return doc.toVariant();
}

SystemComponent* JsonSerializer::parseComponent(const QVariant &variant) const
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    if(!_componentFactories.contains(typeID))
        return nullptr;
    return _componentFactories[typeID](variant);
}

AbstractDataModel* JsonSerializer::parseDataModel(const QVariant &variant) const
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    if(!_modelFactories.contains(typeID))
        return nullptr;
    return _modelFactories[typeID](variant);
}

AbstractPrepareStep* JsonSerializer::parsePrepareStep(const QVariant &variant) const
{
    /* TO BE DONE */

    return nullptr;
}

QJsonObject JsonSerializer::convertVariantToJsonObject(const QVariant &variant) const
{
    QJsonObject ret;

    if(variant.canConvert(QMetaType::QVariantMap))
        ret = QJsonObject::fromVariantMap(variant.toMap());
    else
        qWarning() << "JsonSerializer::serialize: Cannot serialize variant. Incompatible "
                      "information.";

    return ret;
}

} // namespace CTL
