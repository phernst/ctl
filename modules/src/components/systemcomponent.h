#ifndef SYSTEMCOMPONENT_H
#define SYSTEMCOMPONENT_H

#include <QJsonObject>
#include <QString>
#include <memory>
#include "io/jsonserializer.h"

namespace CTL {
/*!
 * \class SystemComponent
 *
 * \brief Base class for all system components.
 *
 * This is the base class for all system components. It holds the components' name and
 * defines the interface type(), elementalType() and info() as well as the clone() method.
 * Additionally, the interface to de-/serialize components from/to JSON format are defined by
 * read() and write().
 *
 * To implement a custom component, create a sub-class of SystemComponent and make sure to
 * register the component in the enumeration using the #ADD_TO_COMPONENT_ENUM(newIndex) macro. It is
 * required to specify a value for \a newIndex that is not already in use. Please refer to the table
 * found in type() for a list of values that are already in use. Another easy way to employ a free
 * index is to use values starting from SystemComponent::UserType, as these are reserved for
 * user-defined types.
 *
 * To provide full compatibility within existing functionality, it is recommended to reimplement the
 * read() and write() method, such that these cover newly introduced information of the sub-class.
 * The new class should then also be added to switch-case list inside the implementation of
 * parseComponentFromJson(const QJsonObject&) found in the header file "components/jsonparser.h".
 *
 * It is strongly advised to sub-class one of the specialized abstract component types (instead of
 * sub-classing SystemComponent directly), with respect to the specific purpose of the new class.
 * By concept, the CTL core module is built-up on four different elemental sub-classes:
 * \li AbstractDetector
 * \li AbstractSource
 * \li AbstractGantry
 * \li AbstractBeamModifier.
 *
 * However, in the rare case that it is desired to extend this selection by a custom elemental
 * component type, SystemComponent can also be sub-classed directly. In this case the
 * #DECLARE_ELEMENTAL_TYPE macro should be used to signalize that the class represents an elemental
 * type. This will provide the possibility to query the underlying elemental type of all further
 * sub-classes using their elementalType() method. Note that newly introduced elemental types will
 * not be considered at any stage within the CTL modules without suitable changes/additions to the
 * corresponding routines.
 *
 * \sa elementalType()
 */

class SystemComponent
{

public:
    enum { Type = 0, UserType = 65536 };

    template <class ModelType>
    struct RegisterToJsonParser
    {
        RegisterToJsonParser();
    };

    // ctors etc.
    SystemComponent(const QString& name = defaultName());
    SystemComponent(const QJsonObject& json); // deprecated
    SystemComponent(const SystemComponent&) = default;
    SystemComponent(SystemComponent&&) = default;
    SystemComponent& operator=(const SystemComponent&) = default;
    SystemComponent& operator=(SystemComponent&&) = default;
    virtual ~SystemComponent() = default;

    // virtual methods
    virtual int type() const;
    virtual int elementalType() const;
    virtual QString info() const;
    virtual SystemComponent* clone() const;

    virtual void fromVariant(const QVariant& variant); // de-serialization
    virtual QVariant toVariant() const; // serialization

    // deprecated
    virtual void read(const QJsonObject& json); // JSON
    virtual void write(QJsonObject& json) const; // JSON

    // getter methods
    const QString& name() const;

    // setter methods
    void rename(QString name);

    // static methods
    static QString defaultName();

protected:
    // methods
    static QString typeInfoString(const std::type_info& type);

private:
    QString _name;
};

// factory function `makeComponent`
template <typename ComponentType, typename... ConstructorArguments>
auto makeComponent(ConstructorArguments&&... arguments) ->
    typename std::enable_if<std::is_convertible<ComponentType*, SystemComponent*>::value,
                            std::unique_ptr<ComponentType>>::type
{
    return std::unique_ptr<ComponentType>(
        new ComponentType(std::forward<ConstructorArguments>(arguments)...));
}

// factory function 'makeComponentFromJson'
std::unique_ptr<SystemComponent> makeComponentFromJson(const QJsonObject& object,
                                                       bool fallbackToGenericType = false);

/*!
 * Returns the type id of the component.
 *
 * List of all default types:
 *
 * Type                        | Type-ID
 * --------------------------- | -------------
 * SystemComponent::Type       |   0
 * AbstractDetector::Type      | 100
 * AbstractGantry::Type        | 200
 * AbstractSource::Type        | 300
 * AbstractBeamModifier::Type  | 400
 * GenericDetector::Type       | 101
 * GenericGantry::Type         | 201
 * GenericSource::Type         | 301
 * GenericBeamModifier::Type   | 401
 * CylindricalDetector::Type   | 110
 * FlatPanelDetector::Type     | 120
 * CarmGantry::Type            | 210
 * TubularGantry::Type         | 220
 * XrayLaser::Type             | 310
 * XrayTube::Type              | 320
 */
inline int SystemComponent::type() const { return Type; }

/*!
 * Returns the type id of the underlying elemental base class.
 *
 * By default, there are four different elemental sub-classes:
 * \li AbstractDetector
 * \li AbstractSource
 * \li AbstractGantry
 * \li AbstractBeamModifier.
 */
inline int SystemComponent::elementalType() const { return Type; }

template<class ModelType>
SystemComponent::RegisterToJsonParser<ModelType>::RegisterToJsonParser()
{
    auto factoryFunction = [](const QVariant& variant) -> SystemComponent*
    {
        auto a = new ModelType();   // requires a default constructor (can also be declared private)
        a->fromVariant(variant);
        return a;
    };
    JsonSerializer::instance().componentFactories().insert(ModelType::Type, factoryFunction);
}

} // namespace CTL

/*! \file */
///@{
/*!
 * \def DECLARE_ELEMENTAL_TYPE
 *
 * This macro defined the component as elemental, i.e. it performs a final override of the
 * elementalType() method, such that calls to this method from sub-classes result in the type id of
 * the elemental type (abstract type).
 */
#define DECLARE_ELEMENTAL_TYPE                                                                     \
public:                                                                                            \
    int elementalType() const final { return Type; }                                               \
                                                                                                   \
private:

/*!
 * \def ADD_TO_COMPONENT_ENUM(newIndex)
 *
 * Macro to add the component to the component enumeration with the index \a newIndex.
 */
#define ADD_TO_COMPONENT_ENUM(newIndex)                                                            \
public:                                                                                            \
    enum { Type = newIndex };                                                                      \
    int type() const override { return Type; }                                                     \
                                                                                                   \
private:                                                                                           \
    template<class>                                                                                \
    friend struct SystemComponent::RegisterToJsonParser;

/*
    template<class ModelType>                                                                      \
    friend SystemComponent::RegisterToJsonParser<ModelType>::RegisterToJsonParser();
*/

/*!
 * \def DECLARE_JSON_COMPATIBLE_COMPONENT(componentClassName_woNamespace)
 *
 * Declares a global variable for a certain system component. Its initialization registers this
 * component to the JsonSerializer. The argument of this macro must be the name of the concrete
 * component that should be registered. The name must not contain any namespace, which can be
 * achieved by using this macro inside the according namespace.
 *
 * The name of the globar variable is `JSON_PARSER_KNOWS_COMP_<componentClassName_woNamespace>`.
 */
#define DECLARE_JSON_COMPATIBLE_COMPONENT(componentClassName_woNamespace)                              \
    CTL::SystemComponent::RegisterToJsonParser<componentClassName_woNamespace>                   \
    JSON_PARSER_KNOWS_COMP_ ## componentClassName_woNamespace;


/*!
 * \fn std::unique_ptr<ComponentType> CTL::makeComponent(ConstructorArguments&&... arguments)
 * \relates SystemComponent
 *
 * Global (free) make function that creates a new SystemComponent from the constructor \a arguments.
 * The component is returned as a `std::unique_ptr<ComponentType>`, whereas `ComponentType` is the
 * template argument of this function that needs to be specified.
 */

/*!
 * \fn std::unique_ptr<SystemComponent> CTL::makeComponentFromJson(const QJsonObject& object,
 *                                                                 bool fallbackToGenericType);
 * \relates SystemComponent
 *
 * Global (free) make function that parses a QJsonObject and creates a concrete SystemComponent (any
 * subtype) whose Type-ID is registered (with ADD_TO_COMPONENT_ENUM).
 * It is also required that this type has been added to the switch-case list inside the
 * implementation of parseComponentFromJson(const QJsonObject&) in the header file
 * "components/jsonparser.h".
 * If the type is not known (not implemented into the switch-case list) and
 * \a fallbackToGenericType is set to `true`, the function tries - by using
 * parseGenericComponentFromJson(const QJsonObject&) - to restore a Generic<Type>
 * (e.g. GenericSource) based of its elemental type id. However in this case
 * all the specific information (member variables) of the unknown type as well as the information of
 * the Generic<Type> cannot be restored, only the information of the elemental Abstract<Type> is
 * recovered.
 * If the parsing of the Json \a object was not successful, the functions returns a `nullptr`.
 */
///@}

#endif // SYSTEMCOMPONENT_H
