#ifndef SERIALIZATIONINTERFACE_H
#define SERIALIZATIONINTERFACE_H

#include "jsonserializer.h"

namespace CTL {

class SerializationInterface
{
public:
    template<class SerializableType>
    struct RegisterWithJsonSerializer
    {
        RegisterWithJsonSerializer();
    };

    virtual void fromVariant(const QVariant& variant); // de-serialization
    virtual QVariant toVariant() const; // serialization

};

template<class SerializableType>
SerializationInterface::RegisterWithJsonSerializer<SerializableType>::RegisterWithJsonSerializer()
{
    auto factoryFunction = [](const QVariant& variant) -> SerializationInterface*
    {
        auto a = new SerializableType();   // requires a default constructor (can also be declared private)
        a->fromVariant(variant);
        return a;
    };
    if(std::is_convertible<SerializableType*, SystemComponent*>::value)
        JsonSerializer::instance().componentFactories().insert(SerializableType::Type, factoryFunction);
    else if(std::is_convertible<SerializableType*, AbstractDataModel*>::value)
        JsonSerializer::instance().modelFactories().insert(SerializableType::Type, factoryFunction);
    else if(std::is_convertible<SerializableType*, AbstractPrepareStep*>::value)
        JsonSerializer::instance().prepareStepFactories().insert(SerializableType::Type, factoryFunction);
}

}

#endif // SERIALIZATIONINTERFACE_H
