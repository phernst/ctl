#ifndef ABSTRACTBEAMMODIFIER_H
#define ABSTRACTBEAMMODIFIER_H

#include "systemcomponent.h"
#include "models/intervaldataseries.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {
/*!
 * \class AbstractBeamModifier
 * \brief Base class for components that modify the X-ray beam.
 *
 * This is the base class for system components that modify the X-ray beam. Possible examples
 * are radiation filters or collimator systems.
 *
 * Sub-classes of AbstractBeamModifier can be created to define all kinds of individual modifiers. A
 * sub-class needs to implement the modify() method, which takes a (constant) reference to the
 * incident radiation spectrum and returns the radition's spectrum afted it passed the beam modifier
 * component. To describe angular dependency of the modification, angles \a theta and \a phi can be
 * passed as optional parameters. These stick to the conventional angle definitions in spherical
 * coordinates (\a phi - azimuthal angle, \a theta - polar angle).
 *
 * When creating a sub-class of AbstractBeamModifier, make sure to register the new component in the
 * enumeration using the #CTL_TYPE_ID(newIndex) macro. It is required to specify a value
 * for \a newIndex that is not already in use. This can be easily achieved by use of values starting
 * from GenericComponent::UserType, as these are reserved for user-defined types.
 *
 * To provide full compatibility within existing functionality, it is recommended to reimplement the
 * read() and write() method, such that these cover newly introduced information of the sub-class.
 * The new class should then also be added to switch-case list inside the implementation of
 * parseComponentFromJson(const QJsonObject&) found in the header file "components/jsonparser.h".
 */
class AbstractBeamModifier : public SystemComponent
{
    CTL_TYPE_ID(400)
    DECLARE_ELEMENTAL_TYPE

    // abstract interface
    public:virtual IntervalDataSeries modify(const IntervalDataSeries& inputSpectrum,
                                            double theta = 0.0,
                                            double phi = 0.0) = 0;

public:
    AbstractBeamModifier(const QString& name = defaultName());

    // virtual methods
    QString info() const override;

    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization
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

#endif // ABSTRACTBEAMMODIFIER_H
