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

/*!
 * \def DECLARE_JSON_COMPATIBLE_TYPE(componentClassName_woNamespace)
 *
 * Declares a global variable for a certain serializable class. Its initialization registers this
 * component with the JsonSerializer. The argument of this macro must be the name of the concrete
 * component that should be registered. The name must not contain any namespace, which can be
 * achieved by using this macro inside the according namespace.
 *
 * The global variable name is `JSON_SERIALIZER_KNOWS_<className_woNamespace>`.
 */
#define DECLARE_JSON_COMPATIBLE_TYPE(className_woNamespace)                        \
    CTL::SerializationInterface::RegisterWithJsonSerializer<className_woNamespace> \
    JSON_SERIALIZER_KNOWS_ ## className_woNamespace;

}

#endif // SERIALIZATIONINTERFACE_H
