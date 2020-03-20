#include "binaryserializer.h"

#include "serializationhelper.h"

#include <QFile>
#include <QDataStream>

namespace CTL {

void BinarySerializer::serialize(const SerializationInterface &serializableObject, const QString &fileName) const
{
    QFile saveFile(fileName);
    if(!saveFile.open(QIODevice::WriteOnly))
    {
        qWarning().noquote() << "BinarySerializer: serializing failed. File(" + fileName +
                                ") could not be opened for writing.";
        return;
    }
    QDataStream out(&saveFile);
    out << serializableObject.toVariant();
    saveFile.close();
}

std::unique_ptr<SystemComponent> BinarySerializer::deserializeComponent(const QString &fileName) const
{
    return std::unique_ptr<SystemComponent>(
        SerializationHelper::parseComponent(variantFromBinaryFile(fileName)));
}

std::unique_ptr<AbstractDataModel> BinarySerializer::deserializeDataModel(const QString &fileName) const
{
    return std::unique_ptr<AbstractDataModel>(
        SerializationHelper::parseDataModel(variantFromBinaryFile(fileName)));
}

std::unique_ptr<AbstractPrepareStep> BinarySerializer::deserializePrepareStep(const QString &fileName) const
{
    return std::unique_ptr<AbstractPrepareStep>(
        SerializationHelper::parsePrepareStep(variantFromBinaryFile(fileName)));
}

std::unique_ptr<AbstractProjector> BinarySerializer::deserializeProjector(const QString& fileName) const
{
    return std::unique_ptr<AbstractProjector>(
                SerializationHelper::parseProjector(variantFromBinaryFile(fileName)));
}

std::unique_ptr<SerializationInterface> BinarySerializer::deserializeMiscObject(const QString &fileName) const
{
    return std::unique_ptr<SerializationInterface>(
                SerializationHelper::parseMiscObject(variantFromBinaryFile(fileName)));
}

std::unique_ptr<AcquisitionSetup> BinarySerializer::deserializeAquisitionSetup(const QString &fileName) const
{
    std::unique_ptr<AcquisitionSetup> ret(new AcquisitionSetup);

    auto variant = variantFromBinaryFile(fileName);
    if(!variant.isValid())
        return nullptr;
    ret->fromVariant(variant);

    return ret;
}

std::unique_ptr<CTsystem> BinarySerializer::deserializeSystem(const QString &fileName) const
{
    std::unique_ptr<CTsystem> ret(new CTsystem);

    auto variant = variantFromBinaryFile(fileName);
    if(!variant.isValid())
        return nullptr;
    ret->fromVariant(variant);

    return ret;
}

QVariant BinarySerializer::variantFromBinaryFile(const QString &fileName)
{
    QVariant ret;

    QFile loadFile(fileName);
    if(!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning().noquote() << "BinarySerializer: deserializing failed. File(" + fileName +
                                ") could not be opened.";
        return ret;
    }

    QDataStream in(&loadFile);
    in >> ret;

    loadFile.close();

    if(!ret.isValid())
        qWarning().noquote() << "BinarySerializer: deserializing failed. File(" + fileName +
                                ") is not a valid binary serialized file.";

    return ret;
}

} // namespace CTL
