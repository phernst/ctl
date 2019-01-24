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
    ADD_TO_COMPONENT_ENUM(401)

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
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // static methods
    static QString defaultName();
};

/*!
 * Constructs a GenericBeamModifier object named \a name.
 */
inline GenericBeamModifier::GenericBeamModifier(const QString& name)
    : AbstractBeamModifier(name)
{
}

inline GenericBeamModifier::GenericBeamModifier(const QJsonObject& json)
    : AbstractBeamModifier(defaultName())
{
    GenericBeamModifier::read(json);
}

inline IntervalDataSeries GenericBeamModifier::modify(const IntervalDataSeries &inputSpectrum, double, double)
{
    return inputSpectrum;
}

/*!
 * Creates a copy of this instance and returns a base-class pointer to the cloned object.
 */
inline SystemComponent* GenericBeamModifier::clone() const { return new GenericBeamModifier(*this); }

/*!
 * Returns a formatted string that contains information about the component.
 */
inline QString GenericBeamModifier::info() const
{
    QString ret(AbstractBeamModifier::info());

    ret += typeInfoString(typeid(this));
    ret += (this->type() == AbstractBeamModifier::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Returns the default name for the component: "Generic beam modifier".
 */
inline QString GenericBeamModifier::defaultName()
{
    static const QString defName(QStringLiteral("Generic beam modifier"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void GenericBeamModifier::read(const QJsonObject &json)
{
    AbstractBeamModifier::read(json);
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
inline void GenericBeamModifier::write(QJsonObject &json) const
{
    AbstractBeamModifier::write(json);
}

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
