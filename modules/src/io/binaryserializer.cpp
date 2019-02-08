#include "binaryserializer.h"

#include <QFile>
#include <QDataStream>

namespace CTL {

void BinarySerializer::serialize(const SerializationInterface &serializableObject, const QString &fileName) const
{
    QFile saveFile(fileName);
    saveFile.open(QIODevice::WriteOnly);
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

std::unique_ptr<SerializationInterface> BinarySerializer::deserializeMiscObject(const QString &fileName) const
{
    return std::unique_ptr<SerializationInterface>(
                SerializationHelper::parseMiscObject(variantFromBinaryFile(fileName)));
}

std::unique_ptr<AcquisitionSetup> BinarySerializer::deserializeAquisitionSetup(const QString &fileName) const
{
    std::unique_ptr<AcquisitionSetup> ret(new AcquisitionSetup);

    auto variant = variantFromBinaryFile(fileName);
    ret->fromVariant(variant);

    return ret;
}

std::unique_ptr<CTsystem> BinarySerializer::deserializeSystem(const QString &fileName) const
{
    std::unique_ptr<CTsystem> ret(new CTsystem);

    auto variant = variantFromBinaryFile(fileName);
    ret->fromVariant(variant);

    return ret;
}

QVariant BinarySerializer::variantFromBinaryFile(const QString &fileName)
{
    QVariant ret;

    QFile loadFile(fileName);
    loadFile.open(QIODevice::ReadOnly);
    QDataStream in(&loadFile);
    in >> ret;
    loadFile.close();

    return ret;
}

} // namespace CTL
