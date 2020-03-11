#include "xraytube.h"
#include "models/xrayspectrummodels.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(XrayTube)

constexpr float DEFAULT_SPECTRUM_BIN_WIDTH = 10;

/*!
 * Constructs an XrayTube with a focal spot size of \a focalSpotSize and its focal spot positioned
 * at \a focalSpotPosition. Sets the tube (acceleration) voltage to \a tubeVoltage and the emission-
 * current-time-product (used for a single X-ray shot) to \a mAs. Also sets the component's name to
 * \a name.
 */
XrayTube::XrayTube(const QSizeF &focalSpotSize,
                   const Vector3x1 &focalSpotPosition,
                   double tubeVoltage,
                   double mAs,
                   const QString &name)
    : AbstractSource(focalSpotSize, focalSpotPosition, new TASMIPSpectrumModel, name)
    , _mAs(mAs)
{
    setTubeVoltage(tubeVoltage);
}

/*!
 * Constructs an XrayTube with a focal spot size of \a focalSpotSize. Sets the tube (acceleration)
 * voltage to \a tubeVoltage and the emission-current-time-product (used for a single X-ray shot)
 * to \a mAs. Also sets the component's name to \a name.
 *
 * This constructor defaults the focal spot position to Vector3x1(0.0).
 */
XrayTube::XrayTube(const QSizeF &focalSpotSize, double tubeVoltage, double mAs, const QString &name)
    : XrayTube(focalSpotSize, Vector3x1(0.0), tubeVoltage, mAs, name)
{
}

/*!
 * Constructs an XrayTube with a tube (acceleration) voltage of \a tubeVoltage and an emission-
 * current-time-product (used for a single X-ray shot) of \a mAs. Also sets the component's name to
 * \a name.
 *
 * This constructor defaults the focal spot size to QSizeF(0.0,0.0) and the focal spot position
 * to Vector3x1(0.0).
 */
XrayTube::XrayTube(double tubeVoltage, double mAs, const QString &name)
    : XrayTube(QSizeF(0.0,0.0), Vector3x1(0.0), tubeVoltage, mAs, name)
{
}

/*!
 * Constructs an XrayTube named \a name.
 *
 * This constructor defaults the focal spot size to QSizeF(0.0,0.0) and the focal spot position
 * to Vector3x1(0.0). The tube (acceleration) voltage is set to 100 keV and the emission-current-
 * time-product (used for a single X-ray shot) to 1.0 mAs.
 */
XrayTube::XrayTube(const QString &name)
    : XrayTube(QSizeF(0.0,0.0), Vector3x1(0.0), 100.0, 1.0, name)
{
}

/*!
 * Returns the nominal photon flux (photons/cm² in 1m distance).
 *
 * This is the product of the emission-current-time-product (mAs) and the intensity constant.
 */
double XrayTube::nominalPhotonFlux() const { return _mAs * _intensityConstant; }

/*!
 * Returns the energy range [in keV] of the radiation emitted by this instance.
 *
 * This is [0 keV, e * tubeVoltage].
 */
EnergyRange XrayTube::nominalEnergyRange() const { return { 0.0f, float(_tubeVoltage) }; }

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class (SystemComponent), the info string contains
 * the following details:
 * \li Tube voltage
 * \li Emission-current-time-product (mAs)
 */
QString XrayTube::info() const
{
    QString ret(AbstractSource::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tTube voltage: " + QString::number(_tubeVoltage) + " keV\n"
        "\tEmission-current-time-product (mAs): " + QString::number(_mAs) + " mAs\n";
    ret += (this->type() == XrayTube::Type) ? "}\n" : "";

    return ret;
}

// use documentation of base class
SystemComponent* XrayTube::clone() const { return new XrayTube(*this); }

/*!
 * Returns the default name for the component: "X-ray tube".
 */
QString XrayTube::defaultName()
{
    const QString defName(QStringLiteral("X-ray tube"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

void XrayTube::updateIntensityConstant()
{
    constexpr auto perMM2toCM2 = 100.0;
    const auto energyRange = nominalEnergyRange();

    // integral of TASMIP data encodes photon flux
    _intensityConstant = _spectrumModel->binIntegral(energyRange.center(),
                                                     energyRange.width()) * perMM2toCM2;

//    const auto nbSpectralBins = std::max({ qRound(nominalEnergyRange().width()), 1 });
//    _intensityConstant = IntervalDataSeries::sampledFromModel(
//                               *_spectrumModel,
//                               nominalEnergyRange().start(),
//                               nominalEnergyRange().end(), nbSpectralBins).integral() * perMM2toCM2;


    qDebug("New intensity constant: %f", _intensityConstant);
}

/*!
 * Returns the tube (acceleration) voltage (in keV).
 */
double XrayTube::tubeVoltage() const { return _tubeVoltage; }

/*!
 * Returns the emission-current-time-product (in mAs) for an individual X-ray shot.
 */
double XrayTube::mAs() const { return _mAs; }

/*!
 * Sets the tube (acceleration) voltage to \a voltage (in keV).
 */
void XrayTube::setTubeVoltage(double voltage)
{
    _tubeVoltage = voltage;

    // now ensured to have spectrum model
//    if(hasSpectrumModel())
//        static_cast<AbstractXraySpectrumModel*>(_spectrumModel.get())->setParameter(_tubeVoltage);

    _spectrumModel->setParameter(_tubeVoltage);
    updateIntensityConstant();
}

/*!
 * Sets the emission-current-time-product [in mAs] (used for a single X-ray shot) to \a mAs.
 */
void XrayTube::setMilliampereSeconds(double mAs) { _mAs = mAs; }


// deprecated, intensity constant derived from TASMIPSpectralModel
//  /*!
//   * Sets the intensity constant to \a value (in PHOTONS / (mAs * cm^2)).
//   */
//  void XrayTube::setIntensityConstant(double value) { _intensityConstant = value; }

// Use SystemComponent::fromVariant() documentation.
void XrayTube::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _tubeVoltage = varMap.value("tube voltage").toDouble();
    _mAs = varMap.value("mAs").toDouble();
}

// Use SerializationInterface::toVariant() documentation.
QVariant XrayTube::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("tube voltage", _tubeVoltage);
    ret.insert("mAs", _mAs);

    return ret;
}

void XrayTube::setSpectrumModel(AbstractXraySpectrumModel*)
{
// deprecated, XrayTube now fixed to TASMIPSpectrumModel
//    AbstractSource::setSpectrumModel(model);

//    if(model)
//        static_cast<AbstractXraySpectrumModel*>(_spectrumModel.get())->setParameter(_tubeVoltage);

    qWarning("Setting spectrum model in XrayTube deprecated, XrayTube now fixed to TASMIPSpectrumModel.");
}

uint XrayTube::spectrumDiscretizationHint() const
{
    const auto ret = int(std::ceil(energyRange().width() / DEFAULT_SPECTRUM_BIN_WIDTH));
    return static_cast<uint>(std::max(ret, 1));
}

} // namespace CTL
