#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include <QVariant>
#include <QMap>
#include <memory>

namespace CTL {

class AbstractDataModel;
class AbstractPrepareStep;
class SystemComponent;

class JsonSerializer
{
public:
    // define tye: pointer to function that creates a data model from a QJsonObject
    typedef AbstractDataModel* (*ModelFactoryFunction)(const QVariant&);
    typedef AbstractPrepareStep* (*PrepareStepFactoryFunction)(const QVariant&);
    typedef SystemComponent* (*ComponentFactoryFunction)(const QVariant&);

    static JsonSerializer& instance();
    // add a model factory function
    QMap<int, ComponentFactoryFunction>& componentFactories();
    QMap<int, ModelFactoryFunction>& modelFactories();
    QMap<int, PrepareStepFactoryFunction>& prepareStepFactories();

    // function that parses the QJsonObject in order to create a concrete system component, data
    // model, or prepare step
    SystemComponent* parseComponent(const QVariant& json) const;
    AbstractDataModel* parseDataModel(const QVariant& variant) const;
    AbstractPrepareStep* parsePrepareStep(const QVariant& json) const;

private:
    // private ctor & non-copyable
    JsonSerializer() = default;
    JsonSerializer(const JsonSerializer&) = delete;
    JsonSerializer& operator=(const JsonSerializer&) = delete;

    // map that maps a type ID to the factory function of a data model
    QMap<int, ComponentFactoryFunction> _componentFactories;
    QMap<int, ModelFactoryFunction> _modelFactories;
    QMap<int, PrepareStepFactoryFunction> _prepareStepFactories;
};

inline JsonSerializer& JsonSerializer::instance()
{
    static JsonSerializer jsonModelParser;
    return jsonModelParser;
}

inline QMap<int, JsonSerializer::ComponentFactoryFunction>& JsonSerializer::componentFactories()
{
    return _componentFactories;
}

inline QMap<int, JsonSerializer::ModelFactoryFunction>& JsonSerializer::modelFactories()
{
    return _modelFactories;
}

inline QMap<int, JsonSerializer::PrepareStepFactoryFunction>& JsonSerializer::prepareStepFactories()
{
    return _prepareStepFactories;
}

inline AbstractDataModel* JsonSerializer::parseDataModel(const QVariant &variant) const
{
    auto varMap = variant.toMap();
    if(!varMap.contains("type-id"))
        return nullptr;
    auto typeID = varMap.value("type-id").toInt();
    return _modelFactories[typeID](variant);
}

} // namespace CTL

#endif // JSONSERIALIZER_H
