#include "jsonserializer.h"

#include "serializationhelper.h"
#include "acquisition/abstractpreparestep.h"
#include "acquisition/ctsystem.h"
#include "components/systemcomponent.h"
#include "models/abstractdatamodel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace CTL {

void JsonSerializer::serialize(const AbstractDataModel &model, const QString &fileName)
{
    serialize(static_cast<const SerializationInterface&>(model), fileName);
}

void JsonSerializer::serialize(const AbstractPrepareStep &prepStep, const QString &fileName)
{
    serialize(static_cast<const SerializationInterface&>(prepStep), fileName);
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

void JsonSerializer::serialize(const CTsystem &system, const QString &fileName)
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::WriteOnly);
    QJsonDocument doc(convertVariantToJsonObject(system.toVariant()));
    saveFile.write(doc.toJson());
    saveFile.close();
}

SystemComponent* JsonSerializer::deserializeComponent(const QString &fileName)
{
    return SerializationHelper::parseComponent(variantFromJsonFile(fileName));
}

AbstractDataModel* JsonSerializer::deserializeDataModel(const QString &fileName)
{
    return SerializationHelper::parseDataModel(variantFromJsonFile(fileName));
}

AbstractPrepareStep* JsonSerializer::deserializePrepareStep(const QString &fileName)
{
    return SerializationHelper::parsePrepareStep(variantFromJsonFile(fileName));
}

CTsystem* JsonSerializer::deserializeSystem(const QString &fileName)
{
    auto ret = new CTsystem();

    auto varMap = variantFromJsonFile(fileName).toMap();

    ret->rename(varMap.value("name").toString());

    QVariantList componentVariantList = varMap.value("components").toList();
    for(const auto& comp : componentVariantList)
        ret->addComponent(SerializationHelper::parseComponent(comp));

    return ret;
}

QVariant JsonSerializer::variantFromJsonFile(const QString &fileName)
{
    QFile loadFile(fileName);
    loadFile.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
    loadFile.close();

    return doc.toVariant();
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

} // namespace CTL
