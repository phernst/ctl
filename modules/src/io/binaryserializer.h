#ifndef CTL_BINARYSERIALIZER_H
#define CTL_BINARYSERIALIZER_H

#include "abstractserializer.h"

namespace CTL {

class BinarySerializer : public AbstractSerializer
{
    // implementation of serialization interface
    public: void serialize(const SerializationInterface& serializableObject,
                           const QString& fileName) const override;


    // implementation of deserialization interface
    public: std::unique_ptr<SystemComponent> deserializeComponent(const QString& fileName) const override;
    public: std::unique_ptr<AbstractDataModel> deserializeDataModel(const QString& fileName) const override;
    public: std::unique_ptr<AbstractPrepareStep> deserializePrepareStep(const QString& fileName) const override;
    public: std::unique_ptr<AbstractProjector> deserializeProjector(const QString& fileName) const override;
    public: std::unique_ptr<SerializationInterface> deserializeMiscObject(const QString& fileName) const override;
    public: std::unique_ptr<AcquisitionSetup> deserializeAquisitionSetup(const QString& fileName) const override;
    public: std::unique_ptr<CTSystem> deserializeSystem(const QString& fileName) const override;

private:
    static QVariant variantFromBinaryFile(const QString& fileName);
};


} // namespace CTL

#endif // CTL_BINARYSERIALIZER_H
