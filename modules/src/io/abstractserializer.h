#ifndef ABSTRACTSERIALIZER_H
#define ABSTRACTSERIALIZER_H

#include "acquisition/abstractpreparestep.h"
#include "acquisition/acquisitionsetup.h"
#include "acquisition/ctsystem.h"
#include "components/systemcomponent.h"
#include "models/abstractdatamodel.h"

namespace CTL {

class AbstractSerializer
{
public:
    // convenience declarations (helps with IDE suggestions)
    void serialize(const AbstractDataModel& model, const QString& fileName) const;
    void serialize(const AbstractPrepareStep& prepStep, const QString& fileName) const;
    void serialize(const AcquisitionSetup& setup, const QString& fileName) const;
    void serialize(const CTsystem& system, const QString& fileName) const;
    void serialize(const SystemComponent& component, const QString& fileName) const;

    // ABSTRACT INTERFACE
    // serialization interface
    public:virtual void serialize(const SerializationInterface& serializableObject,
                                  const QString& fileName) const = 0;

    // deserialization interface
    public:virtual SystemComponent* deserializeComponent(const QString& fileName) const = 0;
    public:virtual AbstractDataModel* deserializeDataModel(const QString& fileName) const = 0;
    public:virtual AbstractPrepareStep* deserializePrepareStep(const QString& fileName) const = 0;
    public:virtual CTsystem* deserializeSystem(const QString& fileName) const = 0;
    public:virtual AcquisitionSetup* deserializeAquisitionSetup(const QString& fileName) const = 0;
};

inline void AbstractSerializer::serialize(const AbstractDataModel& model, const QString& fileName) const
{
    serialize(static_cast<const SerializationInterface&>(model), fileName);
}

inline void AbstractSerializer::serialize(const AbstractPrepareStep& prepStep,
                                   const QString& fileName) const
{
    serialize(static_cast<const SerializationInterface&>(prepStep), fileName);
}

inline void AbstractSerializer::serialize(const AcquisitionSetup& setup, const QString& fileName) const
{
    serialize(static_cast<const SerializationInterface&>(setup), fileName);
}

inline void AbstractSerializer::serialize(const CTsystem& system, const QString& fileName) const
{
    serialize(static_cast<const SerializationInterface&>(system), fileName);
}

inline void AbstractSerializer::serialize(const SystemComponent& component, const QString& fileName) const
{
    serialize(static_cast<const SerializationInterface&>(component), fileName);
}

} // namespace CTL

#endif // ABSTRACTSERIALIZER_H
