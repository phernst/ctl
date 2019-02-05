#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QVariant>
#include <QMap>
#include <memory>

namespace CTL {

class AbstractDataModel;
class AbstractPrepareStep;
class SystemComponent;
class SerializationInterface;

class JsonSerializer
{
public:
    // define types: pointer to function that creates CTL objects from a QVariant
    typedef AbstractDataModel* (*ModelFactoryFunction)(const QVariant&);
    typedef AbstractPrepareStep* (*PrepareStepFactoryFunction)(const QVariant&);
    typedef SystemComponent* (*ComponentFactoryFunction)(const QVariant&);
    typedef SerializationInterface* (*SerializableFactoryFunction)(const QVariant&);

    static JsonSerializer& instance();  // singleton getter

    // maps with registered CTL type factories
    QMap<int, SerializableFactoryFunction>& componentFactories();
    QMap<int, SerializableFactoryFunction>& modelFactories();
    QMap<int, SerializableFactoryFunction>& prepareStepFactories();

    // only for convenience
    static void serialize(const AbstractDataModel& model, const QString& fileName);
    static void serialize(const AbstractPrepareStep& prepStep, const QString& fileName);
    static void serialize(const SystemComponent& component, const QString& fileName);
    // actual serialization method
    static void serialize(const SerializationInterface& serializableObject, const QString& fileName);

    static SystemComponent* deserializeComponent(const QString& fileName);
    static AbstractDataModel* deserializeDataModel(const QString& fileName);
    static AbstractPrepareStep* deserializePrepareStep(const QString& fileName);

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

} // namespace CTL

#endif // JSONSERIALIZER_H
