#ifndef SERIALIZATIONHELPER_H
#define SERIALIZATIONHELPER_H

#include <QVariant>

namespace CTL {

class AbstractDataModel;
class AbstractPrepareStep;
class SystemComponent;
class SerializationInterface;

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

    static SerializationHelper& instance();  // singleton getter

    // maps with registered CTL type factories
    const QMap<int, SerializableFactoryFunction>& componentFactories() const;
    const QMap<int, SerializableFactoryFunction>& modelFactories() const;
    const QMap<int, SerializableFactoryFunction>& prepareStepFactories() const;

    // functions that parse the QVariant in order to create a concrete system component, data
    // model, or prepare step
    static AbstractDataModel* parseDataModel(const QVariant& variant);
    static SystemComponent* parseComponent(const QVariant& variant);
    static AbstractPrepareStep* parsePrepareStep(const QVariant& variant);

private:
    // private ctor & non-copyable
    SerializationHelper() = default;
    SerializationHelper(const SerializationHelper&) = delete;
    SerializationHelper& operator=(const SerializationHelper&) = delete;

    // map that maps a type ID to the factory function of a data model
    QMap<int, SerializableFactoryFunction> _componentFactories;
    QMap<int, SerializableFactoryFunction> _modelFactories;
    QMap<int, SerializableFactoryFunction> _prepareStepFactories;
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

    auto& serializer = SerializationHelper::instance();
    if(std::is_convertible<SerializableType*, SystemComponent*>::value)
    {
        Q_ASSERT(!serializer._componentFactories.contains(SerializableType::Type));
        serializer._componentFactories.insert(SerializableType::Type, factoryFunction);
    }
    else if(std::is_convertible<SerializableType*, AbstractDataModel*>::value)
    {
        Q_ASSERT(!serializer._modelFactories.contains(SerializableType::Type));
        serializer._modelFactories.insert(SerializableType::Type, factoryFunction);
    }
    else if(std::is_convertible<SerializableType*, AbstractPrepareStep*>::value)
    {
        Q_ASSERT(!serializer._prepareStepFactories.contains(SerializableType::Type));
        serializer._prepareStepFactories.insert(SerializableType::Type, factoryFunction);
    }
    else
    {
        Q_ASSERT_X(false, "RegisterWithSerializationHelper", "try a registration of an unknown type");
    }
}

/*!
 * \def DECLARE_SERIALIZABLE_TYPE(className_woNamespace)
 *
 * \relates SerializationHelper
 *
 * Declares a global variable for a certain serializable class. Its initialization registers this
 * class with the SerializationHelper. The argument of this macro must be the name of the concrete
 * class that should be registered. The name must not contain any namespace, which can be
 * achieved by using this macro inside the according namespace.
 *
 * The global variable name is `SERIALIZATION_HELPER_KNOWS_<className_woNamespace>`.
 */
#define DECLARE_SERIALIZABLE_TYPE(className_woNamespace)                                        \
    CTL::SerializationHelper::RegisterWithSerializationHelper<className_woNamespace>                         \
    SERIALIZATION_HELPER_KNOWS_ ## className_woNamespace;


} // namespace CTL

/*! \file */
///@{
///@}

#endif // SERIALIZATIONHELPER_H
