#ifndef CTL_ABSTRACTSERIALIZER_H
#define CTL_ABSTRACTSERIALIZER_H

#include "acquisition/abstractpreparestep.h"
#include "acquisition/acquisitionsetup.h"
#include "acquisition/ctsystem.h"
#include "components/systemcomponent.h"
#include "models/abstractdatamodel.h"
#include "projectors/abstractprojector.h"

namespace CTL {

class AbstractSerializer
{
public:
    virtual ~AbstractSerializer() = default;

    // convenience declarations (helps with IDE suggestions)
    void serialize(const AbstractDataModel& model, const QString& fileName) const;
    void serialize(const AbstractPrepareStep& prepStep, const QString& fileName) const;
    void serialize(const AbstractProjector& prepStep, const QString& fileName) const;
    void serialize(const AcquisitionSetup& setup, const QString& fileName) const;
    void serialize(const CTsystem& system, const QString& fileName) const;
    void serialize(const SystemComponent& component, const QString& fileName) const;

    // ABSTRACT INTERFACE
    // serialization interface
    public:virtual void serialize(const SerializationInterface& serializableObject,
                                  const QString& fileName) const = 0;

    // deserialization interface
    public:virtual std::unique_ptr<SystemComponent> deserializeComponent(const QString& fileName) const = 0;
    public:virtual std::unique_ptr<AbstractDataModel> deserializeDataModel(const QString& fileName) const = 0;
    public:virtual std::unique_ptr<AbstractPrepareStep> deserializePrepareStep(const QString& fileName) const = 0;
    public:virtual std::unique_ptr<AbstractProjector> deserializeProjector(const QString& fileName) const = 0;
    public:virtual std::unique_ptr<CTsystem> deserializeSystem(const QString& fileName) const = 0;
    public:virtual std::unique_ptr<AcquisitionSetup> deserializeAquisitionSetup(const QString& fileName) const = 0;
    public:virtual std::unique_ptr<SerializationInterface> deserializeMiscObject(const QString& fileName) const = 0;

    // convenience deserialization interface for concrete derived types
public:
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserialize(const QString& fileName);

private:
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserializeDerived(const QString& fileName, SystemComponent*);
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserializeDerived(const QString& fileName, AbstractDataModel*);
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserializeDerived(const QString& fileName, AbstractPrepareStep*);
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserializeDerived(const QString& fileName, AbstractProjector*);
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserializeDerived(const QString& fileName, CTsystem*);
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserializeDerived(const QString& fileName, AcquisitionSetup*);
    template<class DerivedType>
    std::unique_ptr<DerivedType> deserializeDerived(const QString& fileName, SerializationInterface*);

protected:
    AbstractSerializer() = default;
    AbstractSerializer(const AbstractSerializer&) = default;
    AbstractSerializer(AbstractSerializer&&) = default;
    AbstractSerializer& operator=(const AbstractSerializer&) = default;
    AbstractSerializer& operator=(AbstractSerializer&&) = default;
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
inline void AbstractSerializer::serialize(const AbstractProjector& projector,
                                          const QString& fileName) const
{
    serialize(static_cast<const SerializationInterface&>(projector), fileName);
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

template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserialize(const QString& fileName)
{
    DerivedType* d = nullptr;
    return deserializeDerived<DerivedType>(fileName, d);
}

template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserializeDerived(const QString& fileName, SystemComponent*)
{
    auto uPtr = deserializeComponent(fileName);
    if(dynamic_cast<DerivedType*>(uPtr.get()))
        return std::unique_ptr<DerivedType>(static_cast<DerivedType*>(uPtr.release()));
    else
        return nullptr;
}
template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserializeDerived(const QString& fileName, AbstractDataModel*)
{
    auto uPtr = deserializeDataModel(fileName);
    if(dynamic_cast<DerivedType*>(uPtr.get()))
        return std::unique_ptr<DerivedType>(static_cast<DerivedType*>(uPtr.release()));
    else
        return nullptr;
}
template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserializeDerived(const QString& fileName, AbstractPrepareStep*)
{
    auto uPtr = deserializePrepareStep(fileName);
    if(dynamic_cast<DerivedType*>(uPtr.get()))
        return std::unique_ptr<DerivedType>(static_cast<DerivedType*>(uPtr.release()));
    else
        return nullptr;
}
template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserializeDerived(const QString& fileName, AbstractProjector*)
{
    auto uPtr = deserializeProjector(fileName);
    if(dynamic_cast<DerivedType*>(uPtr.get()))
        return std::unique_ptr<DerivedType>(static_cast<DerivedType*>(uPtr.release()));
    else
        return nullptr;
}
template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserializeDerived(const QString& fileName, CTsystem*)
{
    auto uPtr = deserializeSystem(fileName);
    if(dynamic_cast<DerivedType*>(uPtr.get()))
        return std::unique_ptr<DerivedType>(static_cast<DerivedType*>(uPtr.release()));
    else
        return nullptr;
}
template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserializeDerived(const QString& fileName, AcquisitionSetup*)
{
    auto uPtr = deserializeAquisitionSetup(fileName);
    if(dynamic_cast<DerivedType*>(uPtr.get()))
        return std::unique_ptr<DerivedType>(static_cast<DerivedType*>(uPtr.release()));
    else
        return nullptr;
}
template<class DerivedType>
std::unique_ptr<DerivedType> AbstractSerializer::deserializeDerived(const QString& fileName, SerializationInterface*)
{
    auto uPtr = deserializeMiscObject(fileName);
    if(dynamic_cast<DerivedType*>(uPtr.get()))
        return std::unique_ptr<DerivedType>(static_cast<DerivedType*>(uPtr.release()));
    else
        return nullptr;
}

} // namespace CTL

#endif // CTL_ABSTRACTSERIALIZER_H
