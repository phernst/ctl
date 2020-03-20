#ifndef CTL_SERIALIZATIONINTERFACE_H
#define CTL_SERIALIZATIONINTERFACE_H

#include "io/serializationhelper.h"

#include <QVariant>

namespace CTL {

/*!
 * \class SerializationInterface
 * \brief Specify an interface for de-/serialization from/to `QVariant`s
 *
 * \sa CTL_TYPE_ID(newIndex)
 */

class SerializationInterface
{
public:
    enum { Type = -1, UserType = 65536 };
    virtual int type() const;
    virtual void fromVariant(const QVariant& variant); // de-serialization
    virtual QVariant toVariant() const; // serialization

    virtual ~SerializationInterface() = default;

protected:
    SerializationInterface() = default;
    SerializationInterface(const SerializationInterface&) = default;
    SerializationInterface(SerializationInterface&&) = default;
    SerializationInterface& operator=(const SerializationInterface&) = default;
    SerializationInterface& operator=(SerializationInterface&&) = default;
};

/*!
 * Returns the type-id of the serializable object. Used in deserialization to determine the proper
 * object type.
 *
 * Add derived classes to the enumeration using the CTL_TYPE_ID macro.
 */
inline int SerializationInterface::type() const
{
    return Type;
}

/*!
 * Interface to read all member variables from the QVariant \a variant.
 *
 * Reimplement this method such that it reads all newly introduced content when sub-classing.
 * A typical reimplementation in sub-classes might look like this:
 * \code
 * DirectBaseClass::fromVariant(variant);
 *
 * // assuming our class has a member "double _myMemberVariable"
 *
 * _myMemberVariable = variant.toMap().value("my member variable").toDouble();
 * \endcode
 */
inline void SerializationInterface::fromVariant(const QVariant&)
{
}

/*!
 * Interface to store all member variables in a QVariant.
 *
 * Stores the object's type-id.
 *
 * Reimplement this method such that it stores all newly introduced object data when
 * sub-classing. This needs to cover everything that is necessary to fully determine the
 * state of an object.
 * Best practice is to invoke the base class version of this method to take care
 * of all content originating from underlying base classes.
 *
 * A typical reimplementation in sub-classes might look like this:
 * \code
 * QVariantMap ret = DirectBaseClass::toVariant().toMap();
 *
 * ret.insert("my member variable", _myMemberVariable);
 *
 * return ret;
 * \endcode
 */
inline QVariant SerializationInterface::toVariant() const
{
    QVariantMap ret;
    ret.insert("type-id", this->type());
    return ret;
}

/*!
 * \fn SerializationInterface::SerializationInterface()
 *
 * Standard default constructor.
 */

/*!
 * \fn SerializationInterface::SerializationInterface(const SerializationInterface&)
 *
 * Standard copy constructor.
 */

/*!
 * \fn SerializationInterface::SerializationInterface(SerializationInterface&&)
 *
 * Standard move constructor.
 */

/*!
 * \fn SerializationInterface& SerializationInterface::operator= (const SerializationInterface&)
 *
 * Standard copy assignment operator.
 */

/*!
 * \fn SerializationInterface& SerializationInterface::operator= (SerializationInterface&&)
 *
 * Standard move assignment operator.
 */

/*!
 * \fn virtual SerializationInterface::~SerializationInterface()
 *
 * Default destructor. Virtual for purpose of polymorphism.
 */

} // namespace CTL

/*! \file */
///@{
/*!
 * \def CTL_TYPE_ID(newIndex)
 *
 * This macro should be used in classes with SerializationInterface as base class.
 * It adds a class to the type enumeration with the index \a newIndex and
 * it overrides the function `int type() const` so that it returns the \a newIndex.
 *
 * Moreover, the `SerializationHelper::RegisterWithSerializationHelper` class is declared as a
 * friend class.
 * This allows access to a possible private default constructor of the class using this macro.
 *
 * The usage of the macro should be in a class definition as follows:
 * \code
 * class MySerializableClass : public SerializationInterface
 * {
 *     CTL_TYPE_ID(42)
 *
 * public:
 *     void fromVariant(const QVariant& variant) override;
 *     QVariant toVariant() const override;
 *
 * // ...
 * };
 * \endcode
 *
 * or further subclasses can be typed like
 * \code
 * class MySerializableSubClass : public MySerializableClass
 * {
 *     CTL_TYPE_ID(43)
 *
 * // possible variant conversions
 * // ...
 * };
 * \endcode
 *
 * Additionally, the type may be registered to the `SerializationHelper` by using the macro
 * `DECLARE_SERIALIZABLE_TYPE(className_woNamespace)`. This is not necessary, however, it enables
 * the SerializationHelper to mangage the deserialization of classes with the following types as
 * base class:
 * - AbstractDataModel
 * - AbstractPrepareStep
 * - SystemComponent
 * - miscellaneous, i.e. none of the above, only SerializationInterface.
 *
 * Note that \a newIndex within one of the top four categories has to be unique for each class that
 * uses this macro. It is not necessary for the \a newIndex to be unique with respect to the other
 * categories.
 *
 * The `SerializationHelper` is used by implementations (subclasses) of `AbstractSerializer`.
 * For instance, you can create an object of the 3rd category from a JSON file with
 * \code
 * auto mySystemComponent = JsonSerializer().deserializeComponent("path/to/mySystemComponent.json").
 * \endcode
 *
 * \sa DECLARE_SERIALIZABLE_TYPE(className_woNamespace)
 */
#define CTL_TYPE_ID(newIndex)                                                                      \
public:                                                                                            \
    enum { Type = (newIndex) };                                                                    \
    int type() const override { return Type; }                                                     \
                                                                                                   \
private:                                                                                           \
    template<class>                                                                                \
    friend struct SerializationHelper::RegisterWithSerializationHelper;
///@}

#endif // CTL_SERIALIZATIONINTERFACE_H
