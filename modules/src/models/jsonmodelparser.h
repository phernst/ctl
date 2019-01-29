#ifndef JSONMODELPARSER_H
#define JSONMODELPARSER_H

#include <QJsonObject>
#include <QMap>

/*
 * NOTE: This is header only.
 */

namespace CTL {

class AbstractDataModel;

class JsonModelParser
{
public:
    // define tye: pointer to function that creates a data model from a QJsonObject
    typedef AbstractDataModel* (*ModelFactoryFunction)(const QJsonObject&);

    static JsonModelParser& instance();

    // add a model factory function
    QMap<int, ModelFactoryFunction>& modelFactories();

    // function that parses the QJsonObject in order to create a concrete data model
    AbstractDataModel* parse(const QJsonObject& object) const;

private:
    // private ctor & non-copyable
    JsonModelParser() = default;
    JsonModelParser(const JsonModelParser&) = delete;
    JsonModelParser& operator=(const JsonModelParser&) = delete;

    // map that maps a type ID to the factory function of a data model
    QMap<int, ModelFactoryFunction> _modelFactories;
};

inline JsonModelParser& JsonModelParser::instance()
{
    static JsonModelParser jsonModelParser;
    return jsonModelParser;
}

inline QMap<int, JsonModelParser::ModelFactoryFunction>& JsonModelParser::modelFactories()
{
    return _modelFactories;
}

inline AbstractDataModel* JsonModelParser::parse(const QJsonObject& object) const
{
    if(!object.contains("type-id"))
        return nullptr;
    auto typeID = object.value("type-id").toInt();
    return _modelFactories[typeID](object);
}

} // namespace CTL

#endif // JSONMODELPARSER_H
