#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QVariant>
#include <QMap>

namespace CTL {

class AbstractDataModel;
class AbstractPrepareStep;
class SystemComponent;
class CTsystem;
class SerializationInterface;

class JsonSerializer
{
public:
    template<class SerializableType>
    struct RegisterWithJsonSerializer
    {
        RegisterWithJsonSerializer();
    };

    // define type: pointer to function that creates CTL objects from a QVariant
    typedef SerializationInterface* (*SerializableFactoryFunction)(const QVariant&);

    static JsonSerializer& instance();  // singleton getter

    // maps with registered CTL type factories
    const QMap<int, SerializableFactoryFunction>& componentFactories() const;
    const QMap<int, SerializableFactoryFunction>& modelFactories() const;
    const QMap<int, SerializableFactoryFunction>& prepareStepFactories() const;

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
    // private ctor & non-copyable
    JsonSerializer() = default;
    JsonSerializer(const JsonSerializer&) = delete;
    JsonSerializer& operator=(const JsonSerializer&) = delete;

    // methods
    static QJsonObject convertVariantToJsonObject(const QVariant& variant);
    void serializeVariant(const QVariant& variant, const QString& fileName) const;
    static QVariant variantFromJsonFile(const QString& fileName);

    // functions that parse the QVariant in order to create a concrete system component, data
    // model, or prepare step
    static SystemComponent* parseComponent(const QVariant& variant);
    static AbstractDataModel* parseDataModel(const QVariant& variant);
    static AbstractPrepareStep* parsePrepareStep(const QVariant& variant);

    // map that maps a type ID to the factory function of a data model
    QMap<int, SerializableFactoryFunction> _componentFactories;
    QMap<int, SerializableFactoryFunction> _modelFactories;
    QMap<int, SerializableFactoryFunction> _prepareStepFactories;
};

template<class SerializableType>
JsonSerializer::RegisterWithJsonSerializer<SerializableType>::RegisterWithJsonSerializer()
{
    auto factoryFunction = [](const QVariant& variant) -> SerializationInterface*
    {
        auto a = new SerializableType();   // requires a default constructor (can also be declared private)
        a->fromVariant(variant);
        return a;
    };
    if(std::is_convertible<SerializableType*, SystemComponent*>::value)
        JsonSerializer::instance()._componentFactories.insert(SerializableType::Type, factoryFunction);
    else if(std::is_convertible<SerializableType*, AbstractDataModel*>::value)
        JsonSerializer::instance()._modelFactories.insert(SerializableType::Type, factoryFunction);
    else if(std::is_convertible<SerializableType*, AbstractPrepareStep*>::value)
        JsonSerializer::instance()._prepareStepFactories.insert(SerializableType::Type, factoryFunction);
}

/*!
 * \def DECLARE_JSON_COMPATIBLE_TYPE(componentClassName_woNamespace)
 *
 * Declares a global variable for a certain serializable class. Its initialization registers this
 * class with the JsonSerializer. The argument of this macro must be the name of the concrete
 * class that should be registered. The name must not contain any namespace, which can be
 * achieved by using this macro inside the according namespace.
 *
 * The global variable name is `JSON_SERIALIZER_KNOWS_<className_woNamespace>`.
 */
#define DECLARE_JSON_COMPATIBLE_TYPE(className_woNamespace)                                        \
    CTL::JsonSerializer::RegisterWithJsonSerializer<className_woNamespace>                         \
    JSON_SERIALIZER_KNOWS_ ## className_woNamespace;

} // namespace CTL

#endif // JSONSERIALIZER_H
