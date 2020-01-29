#ifndef SERIALIZATIONHELPER_H
#define SERIALIZATIONHELPER_H

#include <QMap>

class QVariant;

namespace CTL {

class AbstractDataModel;
class AbstractPrepareStep;
class AbstractProjector;
class SystemComponent;
class SerializationInterface;

/*!
 * \class SerializationHelper
 * \brief Singleton that manages factory functions for parsing `QVariant`(Maps) to CTL types
 *
 * \sa DECLARE_SERIALIZABLE_TYPE(className_woNamespace)
 */
class SerializationHelper
{
public:
    template<class SerializableType>
    struct RegisterWithSerializationHelper
    {
        RegisterWithSerializationHelper();
    };

    // define type: pointer to function that creates CTL objects from a QVariant
    typedef SerializationInterface* (*SerializableFactoryFunction)(const QVariant&);
    typedef QMap<int, SerializableFactoryFunction> FactoryMap;

    static SerializationHelper& instance();  // singleton getter

    // maps with registered CTL type factories
    const FactoryMap& componentFactories() const;
    const FactoryMap& modelFactories() const;
    const FactoryMap& prepareStepFactories() const;
    const FactoryMap& projectorFactories() const;
    const FactoryMap& miscFactories() const;

    // functions that parse the QVariant in order to create a concrete system component, data
    // model, prepare step, or something else with SerializationInterface as base class
    static AbstractDataModel* parseDataModel(const QVariant& variant);
    static SystemComponent* parseComponent(const QVariant& variant);
    static AbstractPrepareStep* parsePrepareStep(const QVariant& variant);
    static AbstractProjector* parseProjector(const QVariant& variant);
    static SerializationInterface* parseMiscObject(const QVariant& variant);

private:
    // private ctor & non-copyable
    SerializationHelper() = default;
    SerializationHelper(const SerializationHelper&) = delete;
    SerializationHelper& operator=(const SerializationHelper&) = delete;

    // generic parse function that uses a specified factory map to create an object
    static SerializationInterface* parse(const QVariant& variant, const FactoryMap& factoryMap);

    // map that maps a type ID to the factory function of a data model
    FactoryMap _componentFactories;
    FactoryMap _modelFactories;
    FactoryMap _prepareStepFactories;
    FactoryMap _miscFactories;
    FactoryMap _projectorFactories;
};

template<class SerializableType>
SerializationHelper::RegisterWithSerializationHelper<SerializableType>::RegisterWithSerializationHelper()
{
    auto factoryFunction = [](const QVariant& variant) -> SerializationInterface*
    {
        auto a = new SerializableType();   // requires a default constructor (can also be declared private)
        a->fromVariant(variant);
        return a;
    };

    constexpr bool sysComponent = std::is_convertible<SerializableType*, SystemComponent*>::value;
    constexpr bool dataModel = std::is_convertible<SerializableType*, AbstractDataModel*>::value;
    constexpr bool prepStep = std::is_convertible<SerializableType*, AbstractPrepareStep*>::value;
    constexpr bool projector = std::is_convertible<SerializableType*, AbstractProjector*>::value;
    constexpr bool misc = std::is_convertible<SerializableType*, SerializationInterface*>::value;

    static_assert(sysComponent || dataModel || prepStep || projector || misc,
                  "RegisterWithSerializationHelper fails: tried a registration of an unknown type");

    auto& serializer = SerializationHelper::instance();
    if(sysComponent)
    {
        Q_ASSERT(!serializer._componentFactories.contains(SerializableType::Type));
        serializer._componentFactories.insert(SerializableType::Type, factoryFunction);
    }
    else if(dataModel)
    {
        Q_ASSERT(!serializer._modelFactories.contains(SerializableType::Type));
        serializer._modelFactories.insert(SerializableType::Type, factoryFunction);
    }
    else if(prepStep)
    {
        Q_ASSERT(!serializer._prepareStepFactories.contains(SerializableType::Type));
        serializer._prepareStepFactories.insert(SerializableType::Type, factoryFunction);
    }
    else if(projector)
    {
        Q_ASSERT(!serializer._projectorFactories.contains(SerializableType::Type));
        serializer._projectorFactories.insert(SerializableType::Type, factoryFunction);
    }
    else if(misc)
    {
        Q_ASSERT(!serializer._miscFactories.contains(SerializableType::Type));
        serializer._miscFactories.insert(SerializableType::Type, factoryFunction);
    }
}

} // namespace CTL

/*! \file */
///@{
/*!
 * \def DECLARE_SERIALIZABLE_TYPE(className_woNamespace)
 *
 * Declares a global variable for a certain serializable class. Its initialization registers this
 * class with the SerializationHelper. The argument of this macro must be the name of the concrete
 * class that should be registered. The name must not contain any namespace, which can be
 * achieved by using this macro inside the according namespace.
 * The macro should be placed in *one* .cpp file only (below the the include statement of the header
 * that defines the class \a className_woNamespace):
 * \code
 * #include "myclass.h"
 *
 * namespace possible_namespace {
 *
 * DECLARE_SERIALIZABLE_TYPE(MyClass);
 * // ...
 * }
 * \endcode
 *
 * This macro can only be applied to classes with `SerializationInterface` as base class.
 * These classes should also used the `CTL_TYPE_ID(id)` macro in their class definitions in order to
 * define a unique type ID within their category, which may be either AbstractDataModel,
 * AbstractPrepareStep, SystemComponent or none of the aforementioned, but at least
 * SerializationInterface as base class.
 *
 * The global variable name is `SERIALIZATION_HELPER_KNOWS_<className_woNamespace>`. Its type is
 * `CTL::SerializationHelper::RegisterWithSerializationHelper<className_woNamespace>`.
 *
 * \sa CTL_TYPE_ID(newIndex)
 */
#define DECLARE_SERIALIZABLE_TYPE(className_woNamespace)                                           \
    ::CTL::SerializationHelper::RegisterWithSerializationHelper<className_woNamespace>             \
    SERIALIZATION_HELPER_KNOWS_ ## className_woNamespace;
///@}

#endif // SERIALIZATIONHELPER_H
