#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QVariant>
#include <QMap>
#include <memory>

namespace CTL {

class AbstractDataModel;
class AbstractPrepareStep;
class SystemComponent;
//class SerializationInterface;

class JsonSerializer
{
public:
    // define types: pointer to function that creates CTL objects from a QVariant
    typedef AbstractDataModel* (*ModelFactoryFunction)(const QVariant&);
    typedef AbstractPrepareStep* (*PrepareStepFactoryFunction)(const QVariant&);
    typedef SystemComponent* (*ComponentFactoryFunction)(const QVariant&);
//    typedef SerializationInterface* (*SerializableFactoryFunction)(const QVariant&);

    static JsonSerializer& instance();  // singleton getter

    // maps with registered CTL type factories
    QMap<int, ComponentFactoryFunction>& componentFactories();
    QMap<int, ModelFactoryFunction>& modelFactories();
    QMap<int, PrepareStepFactoryFunction>& prepareStepFactories();


    void serializeComponent(const SystemComponent& component, const QString& fileName) const;
    void serializeDataModel(const AbstractDataModel& model, const QString& fileName) const;
    void serializePrepareStep(const AbstractPrepareStep& prepStep, const QString& fileName) const;

    SystemComponent* deserializeComponent(const QString& fileName) const;
    AbstractDataModel* deserializeDataModel(const QString& fileName) const;
    AbstractPrepareStep* deserializePrepareStep(const QString& fileName) const;

private:
    // private ctor & non-copyable
    JsonSerializer() = default;
    JsonSerializer(const JsonSerializer&) = delete;
    JsonSerializer& operator=(const JsonSerializer&) = delete;

    // methods
    QJsonObject convertVariantToJsonObject(const QVariant& variant) const;
    void serializeVariant(const QVariant& variant, const QString& fileName) const;
    QVariant variantFromJsonFile(const QString& fileName) const;

    // functions that parse the QVariant in order to create a concrete system component, data
    // model, or prepare step
    SystemComponent* parseComponent(const QVariant& variant) const;
    AbstractDataModel* parseDataModel(const QVariant& variant) const;
    AbstractPrepareStep* parsePrepareStep(const QVariant& variant) const;

    // map that maps a type ID to the factory function of a data model
    QMap<int, ComponentFactoryFunction> _componentFactories;
    QMap<int, ModelFactoryFunction> _modelFactories;
    QMap<int, PrepareStepFactoryFunction> _prepareStepFactories;
};

//class SerializationInterface
//{
//public:
//    template<class ModelType>
//    struct RegisterWithJsonSerializer
//    {
//        RegisterWithJsonSerializer();
//    };

//};

//template<class ModelType>
//SerializationInterface::RegisterWithJsonSerializer<ModelType>::RegisterWithJsonSerializer()
//{
//    auto factoryFunction = [](const QVariant& variant) -> SerializationInterface*
//    {
//        auto a = new ModelType();   // requires a default constructor (can also be declared private)
//        a->fromVariant(variant);
//        return a;
//    };
//    JsonSerializer::instance().componentFactories().insert(ModelType::Type, factoryFunction);
//}

} // namespace CTL

#endif // JSONSERIALIZER_H
