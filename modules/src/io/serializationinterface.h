#ifndef SERIALIZATIONINTERFACE_H
#define SERIALIZATIONINTERFACE_H

#include <QVariant>

namespace CTL {

class SerializationInterface
{
public:
    virtual void fromVariant(const QVariant& variant); // de-serialization
    virtual QVariant toVariant() const; // serialization
};

} // namespace CTL

#endif // SERIALIZATIONINTERFACE_H
