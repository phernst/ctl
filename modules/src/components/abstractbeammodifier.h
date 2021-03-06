#ifndef CTL_ABSTRACTBEAMMODIFIER_H
#define CTL_ABSTRACTBEAMMODIFIER_H

#include "systemcomponent.h"
#include "models/intervaldataseries.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {

// future releases: spatial dependency
// * To describe angular dependency of the modification, angles \a theta and \a phi can be
// * passed as optional parameters. These stick to the conventional angle definitions in spherical
// * coordinates (\a phi - azimuthal angle, \a theta - polar angle).

/*!
 * \class AbstractBeamModifier
 * \brief Base class for components that modify the X-ray beam.
 *
 * This is the base class for system components that modify the X-ray beam. Possible examples
 * are radiation filters or collimator systems.
 *
 * Sub-classes of AbstractBeamModifier can be created to define all kinds of individual modifiers. A
 * sub-class needs to implement the two methods that describe the modification of the spectrum and
 * flux:
 * The modifiedSpectrum() method takes a (constant) reference to the incident radiation spectrum
 * and must return the spectrum afted the radiation passed the beam modifier component.
 * The modifiedFlux() method takes the input flux and a (constant) reference to the incident
 * radiation spectrum and must return the remaining flux behind the beam modifier component.
 *
 * When creating a sub-class of AbstractBeamModifier, make sure to register the new component in the
 * enumeration using the #CTL_TYPE_ID(newIndex) macro. It is required to specify a value
 * for \a newIndex that is not already in use. This can be easily achieved by use of values starting
 * from GenericComponent::UserType, as these are reserved for user-defined types.
 *
 * To enable de-/serialization of objects of the new sub-class, reimplement the toVariant() and
 * fromVariant() methods. These should take care of all newly introduced information of the
 * sub-class. Additionally, call the macro #DECLARE_SERIALIZABLE_TYPE(YourNewClassName) within the
 * .cpp file of your new class (substitute "YourNewClassName" with the actual class name). Objects
 * of the new class can then be de-/serialized with any of the serializer classes (see also
 * AbstractSerializer).
 */
class AbstractBeamModifier : public SystemComponent
{
    CTL_TYPE_ID(400)
    DECLARE_ELEMENTAL_TYPE

    // abstract interface
    public:virtual IntervalDataSeries modifiedSpectrum(const IntervalDataSeries& inputSpectrum) = 0;
    public:virtual double modifiedFlux(double inputFlux,
                                       const IntervalDataSeries& inputSpectrum) = 0;

public:
    // virtual methods
    QString info() const override;

    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    ~AbstractBeamModifier() override = default;

protected:
    AbstractBeamModifier() = default;
    AbstractBeamModifier(const QString& name);

    AbstractBeamModifier(const AbstractBeamModifier&) = default;
    AbstractBeamModifier(AbstractBeamModifier&&) = default;
    AbstractBeamModifier& operator=(const AbstractBeamModifier&) = default;
    AbstractBeamModifier& operator=(AbstractBeamModifier&&) = default;
};

/*!
 * Constructs an AbstractBeamModifier object named \a name.
 */
inline AbstractBeamModifier::AbstractBeamModifier(const QString& name)
    : SystemComponent(name)
{
}

/*!
 * Returns a formatted string that contains information about the component.
 */
inline QString AbstractBeamModifier::info() const
{
    QString ret(SystemComponent::info());

    ret += typeInfoString(typeid(this));
    ret += (this->type() == AbstractBeamModifier::Type) ? "}\n" : "";

    return ret;
}

// Use SerializationInterface::fromVariant() documentation.
inline void AbstractBeamModifier::fromVariant(const QVariant& variant)
{
    SystemComponent::fromVariant(variant);
}

// Use SerializationInterface::toVariant() documentation.
inline QVariant AbstractBeamModifier::toVariant() const
{
    return SystemComponent::toVariant();
}

/*!
 * \fn IntervalDataSeries AbstractBeamModifier::modify(const IntervalDataSeries& inputSpectrum,
 * double theta = 0.0, double phi = 0.0) = 0
 *
 * Returns the modified version of \a inputSpectrum at solid angle position (\a theta, \a phi).
 */

} // namespace CTL

#endif // CTL_ABSTRACTBEAMMODIFIER_H
