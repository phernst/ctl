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
    if(!saveFile.open(QIODevice::WriteOnly))
    {
        qWarning().noquote() << "JsonSerializer: serializing failed. File(" + fileName +
                                ") could not be opened for writing.";
        return;
    }
    QJsonDocument doc(convertVariantToJsonObject(serializableObject.toVariant()));
    saveFile.write(doc.toJson());
    saveFile.close();
}

std::unique_ptr<SystemComponent> JsonSerializer::deserializeComponent(const QString& fileName) const
{
    return std::unique_ptr<SystemComponent>(
        SerializationHelper::parseComponent(variantFromJsonFile(fileName)));
}

std::unique_ptr<AbstractDataModel>
JsonSerializer::deserializeDataModel(const QString& fileName) const
{
    return std::unique_ptr<AbstractDataModel>(
        SerializationHelper::parseDataModel(variantFromJsonFile(fileName)));
}

std::unique_ptr<AbstractPrepareStep>
JsonSerializer::deserializePrepareStep(const QString& fileName) const
{
    return std::unique_ptr<AbstractPrepareStep>(
        SerializationHelper::parsePrepareStep(variantFromJsonFile(fileName)));
}

std::unique_ptr<CTsystem> JsonSerializer::deserializeSystem(const QString& fileName) const
{
    std::unique_ptr<CTsystem> ret(new CTsystem);

    auto variant = variantFromJsonFile(fileName);
    ret->fromVariant(variant);

    return ret;
}

std::unique_ptr<AcquisitionSetup>
JsonSerializer::deserializeAquisitionSetup(const QString& fileName) const
{
    std::unique_ptr<AcquisitionSetup> ret(new AcquisitionSetup);

    auto variant = variantFromJsonFile(fileName);
    ret->fromVariant(variant);

    return ret;
}

std::unique_ptr<SerializationInterface> JsonSerializer::deserializeMiscObject(const QString &fileName) const
{
    return std::unique_ptr<SerializationInterface>(
                SerializationHelper::parseMiscObject(variantFromJsonFile(fileName)));
}

QVariant JsonSerializer::variantFromJsonFile(const QString& fileName)
{
    QFile loadFile(fileName);
    if(!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning().noquote() << "JsonSerializer: deserializing failed. File(" + fileName +
                                ") could not be opened.";
        return QVariant();
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll(), &error);
    loadFile.close();

    if(doc.isNull())
        qWarning().noquote() << "JsonSerializer: deserializing failed. File(" + fileName +
                                ") is not a valid JSON serialized file. Details:"
                             << error.errorString();

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
