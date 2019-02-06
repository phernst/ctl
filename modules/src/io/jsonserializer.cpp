#include "jsonserializer.h"

#include "serializationhelper.h"

#include "acquisition/acquisitionsetup.h"
#include "acquisition/ctsystem.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace CTL {

void JsonSerializer::serialize(const SerializationInterface& serializableObject,
                               const QString& fileName) const
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::WriteOnly);
    QJsonDocument doc(convertVariantToJsonObject(serializableObject.toVariant()));
    saveFile.write(doc.toJson());
    saveFile.close();
}

SystemComponent* JsonSerializer::deserializeComponent(const QString& fileName) const
{
    return SerializationHelper::parseComponent(variantFromJsonFile(fileName));
}

AbstractDataModel* JsonSerializer::deserializeDataModel(const QString& fileName) const
{
    return SerializationHelper::parseDataModel(variantFromJsonFile(fileName));
}

AbstractPrepareStep* JsonSerializer::deserializePrepareStep(const QString& fileName) const
{
    return SerializationHelper::parsePrepareStep(variantFromJsonFile(fileName));
}

CTsystem* JsonSerializer::deserializeSystem(const QString& fileName) const
{
    auto ret = new CTsystem;

    auto variant = variantFromJsonFile(fileName);
    ret->fromVariant(variant);

    return ret;
}

AcquisitionSetup* JsonSerializer::deserializeAquisitionSetup(const QString& fileName) const
{
    auto ret = new AcquisitionSetup;

    auto variant = variantFromJsonFile(fileName);
    ret->fromVariant(variant);

    return ret;
}

QVariant JsonSerializer::variantFromJsonFile(const QString& fileName)
{
    QFile loadFile(fileName);
    loadFile.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
    loadFile.close();

    return doc.toVariant();
}

QJsonObject JsonSerializer::convertVariantToJsonObject(const QVariant& variant)
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
