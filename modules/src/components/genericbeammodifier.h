#ifndef GENERICBEAMMODIFIER_H
#define GENERICBEAMMODIFIER_H

#include "abstractbeammodifier.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {
/*!
 * \class GenericBeamModifier
 *
 * \brief Base class for components that modify the X-ray beam.
 *
 * This is a placeholder class for a generic implementation of AbstractBeamModifier.
 * Currently, this does not modify the incident spectrum at all.
 */
class GenericBeamModifier : public AbstractBeamModifier
{
    CTL_TYPE_ID(401)

    // implementation of abstract interface
    public: IntervalDataSeries modify(const IntervalDataSeries& inputSpectrum,
                                     double theta = 0.0,
                                     double phi = 0.0) override;

public:
    GenericBeamModifier(const QString& name = defaultName());
    GenericBeamModifier(const QJsonObject& json);

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // deprecated
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // static methods
    static QString defaultName();
};

} // namespace CTL

/*! \file */
///@{
/*!
 * \fn std::unique_ptr<GenericBeamModifier> CTL::makeModifier(ConstructorArguments&&... arguments)
 * \relates GenericBeamModifier
 *
 * Factory method to construct a GenericBeamModifier and wrap the object in a
 * std::unique_ptr<GenericBeamModifier>.
 *
 * This is similar to the more general method GenericComponent::makeComponent() with the difference
 * that it returns a unique pointer to the GenericBeamModifier base type instead of
 * GenericComponent.
 *
 * \sa GenericComponent::makeComponent().
 */
///@}

#endif // GENERICBEAMMODIFIER_H
