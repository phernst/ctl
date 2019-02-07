#ifndef SERIALIZATIONINTERFACE_H
#define SERIALIZATIONINTERFACE_H

#include "io/serializationhelper.h"

namespace CTL {

class SerializationInterface
{
public:
    enum { Type = -1, UserType = 65536 };
    virtual int type() const;
    virtual void fromVariant(const QVariant& variant); // de-serialization
    virtual QVariant toVariant() const; // serialization

    SerializationInterface() = default;
    SerializationInterface(const SerializationInterface&) = default;
    SerializationInterface(SerializationInterface&&) = default;
    SerializationInterface& operator= (const SerializationInterface&) = default;
    SerializationInterface& operator= (SerializationInterface&&) = default;
    virtual ~SerializationInterface() = default;
};

inline int SerializationInterface::type() const
{
    return Type;
}

inline void SerializationInterface::fromVariant(const QVariant&)
{
}

inline QVariant SerializationInterface::toVariant() const
{
    return QVariant();
}

} // namespace CTL

/*! \file */
///@{
/*!
 * \def CTL_TYPE_ID(newIndex)
 *
 * \relates SerializationInterface
 *
 * This macro should be used in classes with SerializationInterface as base class.
 * It adds a class to the type enumeration with the index \a newIndex and
 * it overrides the function `int type() const` so that it returns the \a newIndex.
 *
 * Moreover, the `JsonSerializer::RegisterWithJsonSerializer` class is declared a as a firend class.
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
 * class MySubSerializableClass : public MySerializableClass
 * {
 *     CTL_TYPE_ID(43)
 * // ...
 * };
 * \endcode
 *
 * The SerializationHelper can mangage the serialization of classes with the following types as base
 * class:
 * - AbstractDataModel
 * - AbstractPrepareStep
 * - SystemComponent
 * - none of the above, only SerializationInterface
 * Note that \a newIndex within one of the top four categories has to be unique for each class that
 * uses this macro. It is not necessary for the \a newIndex to be unique with respect to the other
 * categories.
 */
#define CTL_TYPE_ID(newIndex)                                                                      \
public:                                                                                            \
    enum { Type = newIndex };                                                                      \
    int type() const override { return Type; }                                                     \
                                                                                                   \
private:                                                                                           \
    template<class>                                                                                \
    friend struct SerializationHelper::RegisterWithSerializationHelper;
///@}

#endif // SERIALIZATIONINTERFACE_H
