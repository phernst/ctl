#ifndef CTL_JSONSERIALIZER_H
#define CTL_JSONSERIALIZER_H

#include "abstractserializer.h"

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
    public: std::unique_ptr<AbstractProjector> deserializeProjector(const QString& fileName) const override;
    public: std::unique_ptr<CTSystem> deserializeSystem(const QString& fileName) const override;
    public: std::unique_ptr<AcquisitionSetup> deserializeAquisitionSetup(const QString& fileName) const override;
    public: std::unique_ptr<SerializationInterface> deserializeMiscObject(const QString& fileName) const override;

    static QVariant variantFromJsonFile(const QString& fileName);
private:
    // methods
    static QJsonObject convertVariantToJsonObject(const QVariant& variant);
};

} // namespace CTL

/*! \file */
///@{
///@}

#endif // CTL_JSONSERIALIZER_H
