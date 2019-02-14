#include "xraytube.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(XrayTube)

/*!
 * Constructs an XrayTube with a focal spot size of \a focalSpotSize and its focal spot positioned
 * at \a focalSpotPosition. Sets the tube (acceleration) voltage to \a tubeVoltage and the emission
 * current to \a emissionCurrent. Also sets the component's name to \a name.
 */
XrayTube::XrayTube(const QSizeF &focalSpotSize,
                   const Vector3x1 &focalSpotPosition,
                   double tubeVoltage,
                   double emissionCurrent,
                   const QString &name)
    : AbstractSource(focalSpotSize, focalSpotPosition, new KramersLawSpectrumModel, name),
      _tubeVoltage(tubeVoltage),
      _emissionCurrent(emissionCurrent)
{
}

/*!
 * Constructs an XrayTube with a focal spot size of \a focalSpotSize. Sets the tube (acceleration)
 * voltage to \a tubeVoltage and the emission current to \a emissionCurrent. Also sets the
 * component's name to \a name.
 *
 * This constructor defaults the focal spot position to Vector3x1(0.0).
 */
XrayTube::XrayTube(const QSizeF &focalSpotSize, double tubeVoltage, double emissionCurrent, const QString &name)
    : XrayTube(focalSpotSize, Vector3x1(0.0), tubeVoltage, emissionCurrent, name)
{
}

/*!
 * Constructs an XrayTube with a tube (acceleration) voltage of \a tubeVoltage and an emission
 * current of \a emissionCurrent. Also sets the component's name to \a name.
 *
 * This constructor defaults the focal spot size to QSizeF(0.0,0.0) and the focal spot position
 * to Vector3x1(0.0).
 */
XrayTube::XrayTube(double tubeVoltage, double emissionCurrent, const QString &name)
    : XrayTube(QSizeF(0.0,0.0), Vector3x1(0.0), tubeVoltage, emissionCurrent, name)
{
}

/*!
 * Constructs an XrayTube named \a name.
 *
 * This constructor defaults the focal spot size to QSizeF(0.0,0.0) and the focal spot position
 * to Vector3x1(0.0). The tube (acceleration) voltage is set to 100 keV and the emission current to
 * 1.0 mA.
 */
XrayTube::XrayTube(const QString &name)
    : XrayTube(QSizeF(0.0,0.0), Vector3x1(0.0), 100.0f, 1.0f, name)
{
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class (SystemComponent), the info string contains
 * the following details:
 * \li Tube voltage
 * \li Emission current
 */
QString XrayTube::info() const
{
    QString ret(AbstractSource::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tTube voltage: " + QString::number(_tubeVoltage) + " keV\n"
        "\tEmission current: " + QString::number(_emissionCurrent) + " mA\n";
    ret += (this->type() == XrayTube::Type) ? "}\n" : "";

    return ret;
}

IntervalDataSeries XrayTube::spectrum() const
{
    return spectrum(0.0f, _tubeVoltage, DEFAULT_SPECTRUM_RESOLUTION);
}

/*!
 * Returns the default name for the component: "X-ray tube".
 */
QString XrayTube::defaultName()
{
    static const QString defName(QStringLiteral("X-ray tube"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * Returns the emitted radiation spectrum sampled with \a nbSamples bins covering the energy
 * range of [\a from, \a to] keV. Each energy bin is defined to represent the integral over the
 * contribution of all energies within the bin to the total intensity. The spectrum provides
 * relative intensities, i.e. the sum over all bins equals to one.
 *
 * Throws std::runtime_error if no spectrum model is available.
 */
IntervalDataSeries XrayTube::spectrum(float from, float to, uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");
    static_cast<AbstractXraySpectrumModel*>(_spectrumModel.get())->setParameter(_tubeVoltage);
    IntervalDataSeries spec = IntervalDataSeries::sampledFromModel(*spectrumModel(), from, to, nbSamples);
    spec.normalizeByIntegral();
    return spec;
}

/*!
 * Returns the nominal photon flux (photons/cm² in 1m distance).
 *
 * This is the product of the emission current and the intensity constant.
 */
double XrayTube::nominalPhotonFlux() const
{
    return _emissionCurrent * _intensityConstant;
}

// use documentation of base class
SystemComponent* XrayTube::clone() const { return new XrayTube(*this); }

/*!
 * Returns the tube (acceleration) voltage (in keV).
 */
double XrayTube::tubeVoltage() const { return _tubeVoltage; }

/*!
 * Returns the emission current (in mA).
 */
double XrayTube::emissionCurrent() const { return _emissionCurrent; }

/*!
 * Sets the tube (acceleration) voltage to \a voltage (in keV).
 */
void XrayTube::setTubeVoltage(double voltage) { _tubeVoltage = voltage; }

/*!
 * Sets the emission current to \a current (in mA).
 */
void XrayTube::setEmissionCurrent(double current) { _emissionCurrent = current; }

/*!
 * Sets the intensity constant to \a value (in PHOTONS / (mAs * cm^2)).
 */
void XrayTube::setIntensityConstant(double value) { _intensityConstant = value; }

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void XrayTube::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _tubeVoltage = varMap.value("tube voltage").toDouble();
    _emissionCurrent = varMap.value("emission current").toDouble();
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant XrayTube::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("tube voltage", _tubeVoltage);
    ret.insert("emission current", _emissionCurrent);

    return ret;
}

} // namespace CTL
