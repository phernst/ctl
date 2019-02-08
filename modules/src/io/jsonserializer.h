#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include "abstractserializer.h"

#include <QVariant>

namespace CTL {

class JsonSerializer : public AbstractSerializer
{
    // implementation of serialization interface
    public: void serialize(const SerializationInterface& serializableObject,
                           const QString& fileName) const override;

    // implementation of deserialization interface
    public: std::unique_ptr<SystemComponent> deserializeComponent(const QString& fileName) const override;
    public: std::unique_ptr<AbstractDataModel> deserializeDataModel(const QString& fileName) const override;
    public: std::unique_ptr<AbstractPrepareStep> deserializePrepareStep(const QString& fileName) const override;
    public: std::unique_ptr<CTsystem> deserializeSystem(const QString& fileName) const override;
    public: std::unique_ptr<AcquisitionSetup> deserializeAquisitionSetup(const QString& fileName) const override;
    public: std::unique_ptr<SerializationInterface> deserializeMiscObject(const QString& fileName) const override;

private:
    // methods
    static QJsonObject convertVariantToJsonObject(const QVariant& variant);
    static QVariant variantFromJsonFile(const QString& fileName);
};

} // namespace CTL

/*! \file */
///@{
///@}

#endif // JSONSERIALIZER_H
