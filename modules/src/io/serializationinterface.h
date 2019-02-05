#ifndef SERIALIZATIONINTERFACE_H
#define SERIALIZATIONINTERFACE_H

#include "jsonserializer.h"

namespace CTL {

class SerializationInterface
{
public:
    template<class ModelType>
    struct RegisterWithJsonSerializer
    {
        RegisterWithJsonSerializer();
    };

    virtual void fromVariant(const QVariant& variant); // de-serialization
    virtual QVariant toVariant() const; // serialization

};

template<class ModelType>
SerializationInterface::RegisterWithJsonSerializer<ModelType>::RegisterWithJsonSerializer()
{
    auto factoryFunction = [](const QVariant& variant) -> SerializationInterface*
    {
        auto a = new ModelType();   // requires a default constructor (can also be declared private)
        a->fromVariant(variant);
        return a;
    };
    if(std::is_convertible<ModelType*, SystemComponent*>::value)
        JsonSerializer::instance().componentFactories().insert(ModelType::Type, factoryFunction);
    else if(std::is_convertible<ModelType*, AbstractDataModel*>::value)
        JsonSerializer::instance().modelFactories().insert(ModelType::Type, factoryFunction);
    else if(std::is_convertible<ModelType*, AbstractPrepareStep*>::value)
        JsonSerializer::instance().prepareStepFactories().insert(ModelType::Type, factoryFunction);
}

}

#endif // SERIALIZATIONINTERFACE_H
