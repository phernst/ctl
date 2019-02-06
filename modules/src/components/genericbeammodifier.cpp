#include "genericbeammodifier.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(GenericBeamModifier)

/*!
 * Constructs a GenericBeamModifier object named \a name.
 */
GenericBeamModifier::GenericBeamModifier(const QString& name)
    : AbstractBeamModifier(name)
{
}

GenericBeamModifier::GenericBeamModifier(const QJsonObject& json)
    : AbstractBeamModifier(defaultName())
{
    GenericBeamModifier::read(json);
}

IntervalDataSeries GenericBeamModifier::modify(const IntervalDataSeries &inputSpectrum, double, double)
{
    return inputSpectrum;
}

/*!
 * Creates a copy of this instance and returns a base-class pointer to the cloned object.
 */
SystemComponent* GenericBeamModifier::clone() const { return new GenericBeamModifier(*this); }

/*!
 * Returns a formatted string that contains information about the component.
 */
QString GenericBeamModifier::info() const
{
    QString ret(AbstractBeamModifier::info());

    ret += typeInfoString(typeid(this));
    ret += (this->type() == AbstractBeamModifier::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Returns the default name for the component: "Generic beam modifier".
 */
QString GenericBeamModifier::defaultName()
{
    static const QString defName(QStringLiteral("Generic beam modifier"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void GenericBeamModifier::read(const QJsonObject &json)
{
    AbstractBeamModifier::read(json);
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
void GenericBeamModifier::write(QJsonObject &json) const
{
    AbstractBeamModifier::write(json);
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void GenericBeamModifier::fromVariant(const QVariant& variant)
{
    AbstractBeamModifier::fromVariant(variant);
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant GenericBeamModifier::toVariant() const
{
    return AbstractBeamModifier::toVariant();
}

} // namespace CTL
