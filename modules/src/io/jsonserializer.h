#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QVariant>

namespace CTL {

class AbstractDataModel;
class AbstractPrepareStep;
class CTsystem;
class SerializationInterface;
class SystemComponent;

class JsonSerializer
{
public:
    // only for convenience
    static void serialize(const AbstractDataModel& model, const QString& fileName);
    static void serialize(const AbstractPrepareStep& prepStep, const QString& fileName);
    static void serialize(const SystemComponent& component, const QString& fileName);
    // actual serialization method
    static void serialize(const SerializationInterface& serializableObject, const QString& fileName);

    static void serialize(const CTsystem& system, const QString& fileName);

    static SystemComponent* deserializeComponent(const QString& fileName);
    static AbstractDataModel* deserializeDataModel(const QString& fileName);
    static AbstractPrepareStep* deserializePrepareStep(const QString& fileName);
    static CTsystem* deserializeSystem(const QString& fileName);

private:
    // methods
    static QJsonObject convertVariantToJsonObject(const QVariant& variant);
    void serializeVariant(const QVariant& variant, const QString& fileName) const;
    static QVariant variantFromJsonFile(const QString& fileName);
};

} // namespace CTL

/*! \file */
///@{
///@}

#endif // JSONSERIALIZER_H
