#include "xraylaser.h"
#include "models/xrayspectrummodels.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(XrayLaser)

/*!
 * Constructs an XrayLaser with a focal spot size of \a focalSpotSize and its focal spot positioned
 * at \a focalSpotPosition. Sets the energy of emitted photons to \a energy [in keV] and the total
 * emitted output to \a output [in mWs]. Also sets the component's name to \a name.
 */
XrayLaser::XrayLaser(const QSizeF &focalSpotSize,
                     const Vector3x1 &focalSpotPosition,
                     double energy,
                     double output,
                     const QString &name)
    : AbstractSource(focalSpotSize, focalSpotPosition, new XrayLaserSpectrumModel, name)
    , _output(output)
{
    setPhotonEnergy(energy);
}

/*!
 * Constructs an XrayLaser that emits photons with an energy of \a energy and a total emitted
 * output of \a output [in mWs]. Also sets the component's name to \a name.
 *
 * The focal spot size defaults to QSizeF(0.0,0.0) and the focal spot position is set to
 * Vector3x1(0.0f).
 */
XrayLaser::XrayLaser(double energy, double output, const QString &name)
    : XrayLaser(QSizeF(0.0,0.0), Vector3x1(0.0f), energy, output, name)
{
}

/*!
 * Constructs an XrayLaser named \a name.
 *
 * The focal spot size defaults to QSizeF(0.0,0.0) and the focal spot position is set to
 * Vector3x1(0.0f). Sets the photon energy to 100 keV and the total emitted output to 1.0 mWs.
 */
XrayLaser::XrayLaser(const QString &name)
    : XrayLaser(QSizeF(0.0,0.0), Vector3x1(0.0f), 100.0f, 1.0f, name)
{
}

/*!
 * Returns the nominal photon flux (photons/cm² in 1m distance).
 *
 * This is computed as the quotient between the total emitted output and the energy of an individual
 * photon.
 */
double XrayLaser::nominalPhotonFlux() const { return _output / (ELEC_VOLT * photonEnergy() * 1.0e3); }

/*!
 * Returns the energy range [in keV] of the radiation emitted by this instance.
 *
 * This is [photonEnergy, photonEnergy].
 */
AbstractSource::EnergyRange XrayLaser::energyRange() const
{
    return { float(_energy), float(_energy) };
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class (SystemComponent), the info string contains
 * the following details:
 * \li Energy of emitted photons
 * \li Total emitted radiation output
 */
QString XrayLaser::info() const
{
    QString ret(AbstractSource::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tEnergy: " + QString::number(_energy) + " keV\n"
        "\tOutput: " +  QString::number(_output) + " mWs\n";
    ret += (this->type() == XrayLaser::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Returns a hint for a reasonable number of sampling points when querying a spectrum of the
 * component. This always returns 1, since a single energy bin is sufficient to represent
 * monochromatic radiation.
 */
uint XrayLaser::spectrumDiscretizationHint() const
{
    return 1;
}

// Use SystemComponent::fromVariant() documentation.
void XrayLaser::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _energy = varMap.value("energy").toDouble();
    _output  = varMap.value("output").toDouble();
}

// Use SerializationInterface::toVariant() documentation.
QVariant XrayLaser::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("energy", _energy);
    ret.insert("output", _output);

    return ret;
}

/*!
 * Returns the default name for the component: "X-ray laser".
 */
QString XrayLaser::defaultName()
{
    static const QString defName(QStringLiteral("X-ray laser"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

// Use the documentation of base class.
SystemComponent* XrayLaser::clone() const { return new XrayLaser(*this); }


/*!
 * Returns the energy of emitted photons (in keV).
 */
double XrayLaser::photonEnergy() const { return _energy; }

/*!
 * Returns the total emission output (in mWs). This refers to all radiation that is emitted to an
 * area of 1cm² in a distance of 1m from the source.
 */
double XrayLaser::radiationOutput() const { return _output; }

/*!
 * Sets the energy of emitted photons to \a energy (in keV).
 */
void XrayLaser::setPhotonEnergy(double energy)
{
    _energy = energy;

    if(hasSpectrumModel())
        static_cast<AbstractXraySpectrumModel*>(_spectrumModel.get())->setParameter(_energy);
}

/*!
 * Sets the total radiation output emitted to an area of 1cm² in a distance of 1m to \a output
 * (in mWs).
 */
void XrayLaser::setRadiationOutput(double output) { _output = output; }

} // namespace CTL
