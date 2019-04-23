#include "xraylaser.h"
#include "models/xrayspectrummodels.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(XrayLaser)

/*!
 * Constructs an XrayLaser with a focal spot size of \a focalSpotSize and its focal spot positioned
 * at \a focalSpotPosition. Sets the energy of emitted photons to \a energy and the total emitted
 * power to \a power. Also sets the component's name to \a name.
 */
XrayLaser::XrayLaser(const QSizeF &focalSpotSize,
                     const Vector3x1 &focalSpotPosition,
                     double energy,
                     double power,
                     const QString &name)
    : AbstractSource(focalSpotSize, focalSpotPosition, new XrayLaserSpectrumModel, name)
    , _energy(energy)
    , _power(power)
{
}

/*!
 * Constructs an XrayLaser that emits photons with an energy of \a energy and a total emitted
 * power of \a power. Also sets the component's name to \a name.
 *
 * The focal spot size defaults to QSizeF(0.0,0.0) and the focal spot position is set to
 * Vector3x1(0.0f).
 */
XrayLaser::XrayLaser(double energy, double power, const QString &name)
    : XrayLaser(QSizeF(0.0,0.0), Vector3x1(0.0f), energy, power, name)
{
}

/*!
 * Constructs an XrayLaser named \a name.
 *
 * The focal spot size defaults to QSizeF(0.0,0.0) and the focal spot position is set to
 * Vector3x1(0.0f). Sets the photon energy to 100 keV and the total power to 1.0.
 */
XrayLaser::XrayLaser(const QString &name)
    : XrayLaser(QSizeF(0.0,0.0), Vector3x1(0.0f), 100.0f, 1.0f, name)
{
}

/*!
 * Returns the emitted radiation spectrum sampled with \a nbSamples bins covering the energy
 * range of [\a from, \a to] keV. Each energy bin is defined to represent the integral over the
 * contribution of all energies within the bin to the total intensity. The spectrum provides
 * relative intensities, i.e. the sum over all bins equals to one.
 *
 * Throws std::runtime_error if no spectrum model is available.
 */
IntervalDataSeries XrayLaser::spectrum(float from, float to, uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");

    static_cast<XrayLaserSpectrumModel*>(_spectrumModel.get())->setParameter(_energy);
    IntervalDataSeries spec = IntervalDataSeries::sampledFromModel(*spectrumModel(), from, to, nbSamples);
    spec.normalizeByIntegral();
    return spec;
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class (SystemComponent), the info string contains
 * the following details:
 * \li Energy of emitted photons
 * \li Total emitted power
 */
QString XrayLaser::info() const
{
    QString ret(AbstractSource::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tEnergy: " + QString::number(_energy) + " keV\n"
        "\tPower: " +  QString::number(_power) + "\n";
    ret += (this->type() == XrayLaser::Type) ? "}\n" : "";

    return ret;
}

// Use SystemComponent::fromVariant() documentation.
void XrayLaser::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _energy = varMap.value("energy").toDouble();
    _power  = varMap.value("power").toDouble();
}

// Use SerializationInterface::toVariant() documentation.
QVariant XrayLaser::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("energy", _energy);
    ret.insert("power", _power);

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
 * Returns the nominal photon flux.
 *
 * Currently, simply returns the total power.
 */
double XrayLaser::nominalPhotonFlux() const { return _power; }

/*!
 * Returns the energy of emitted photons (in keV).
 */
double XrayLaser::photonEnergy() const { return _energy; }

/*!
 * Sets the energy of emitted photons to \a energy (in keV).
 */
void XrayLaser::setPhotonEnergy(double energy) { _energy = energy; }

/*!
 * Sets the total emitted power to \a power.
 */
void XrayLaser::setPower(double power) { _power = power; }

} // namespace CTL
