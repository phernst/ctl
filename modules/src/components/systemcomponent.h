#ifndef CTL_SYSTEMCOMPONENT_H
#define CTL_SYSTEMCOMPONENT_H

#include "io/serializationinterface.h"
#include <QJsonObject>
#include <QString>
#include <memory>

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
 * register the component in the enumeration using the #CTL_TYPE_ID(newIndex) macro. It is
 * required to specify a value for \a newIndex that is not already in use. Please refer to the table
 * found in type() for a list of values that are already in use. Another easy way to employ a free
 * index is to use values starting from SystemComponent::UserType, as these are reserved for
 * user-defined types.
 *
 * To enable de-/serialization of objects of the new sub-class, reimplement the toVariant() and
 * fromVariant() methods. These should take care of all newly introduced information of the
 * sub-class. Additionally, call the macro #DECLARE_SERIALIZABLE_TYPE(YourNewClassName) within the
 * .cpp file of your new class (substitute "YourNewClassName" with the actual class name). Objects
 * of the new class can then be de-/serialized with any of the serializer classes (see also
 * AbstractSerializer).
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

class SystemComponent : public SerializationInterface
{
    CTL_TYPE_ID(0)

public:
    // ctors etc.
    SystemComponent(const QString& name = defaultName());

    // virtual methods
    virtual int elementalType() const;
    virtual QString info() const;
    virtual SystemComponent* clone() const;

    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

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
    QString _name; //!< The component's name.
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
 * \fn int SystemComponent::type() const
 *
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
 * subtype) whose Type-ID is registered (with CTL_TYPE_ID).
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

#endif // CTL_SYSTEMCOMPONENT_H
