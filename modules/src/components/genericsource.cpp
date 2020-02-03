#include "genericsource.h"
#include <QDebug>
#include <stdexcept>
#include "acquisition/simplectsystem.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(GenericSource)

/*!
 * Constructs a GenericSource with the name \a name.
 *
 * Focal spot size defaults to QSizeF(0.0, 0.0) and the focal spot position to Vector3x1(0.0f).
 */
GenericSource::GenericSource(const QString& name)
    : AbstractSource(name)
{
}

GenericSource::GenericSource(const IntervalDataSeries& spectrum, double photonFlux,
                             const QSizeF& focalSpotSize, const Vector3x1& focalSpotPosition,
                             const QString& name)
    : GenericSource(focalSpotSize, focalSpotPosition, name)
{
    setSpectrum(spectrum, true);
    if(photonFlux > 0.0)
        setPhotonFlux(photonFlux);
}

/*!
 * Constructs a GenericSource with a focal spot size of \a focalSpotSize, the focal spot positioned
 * at \a focalSpotPosition and the name \a name.
 */
GenericSource::GenericSource(const QSizeF& focalSpotSize,
                             const Vector3x1& focalSpotPosition,
                             const QString& name)
    : AbstractSource(focalSpotSize, focalSpotPosition, name)
{
}

/*!
 * Returns the default name for the component: "Generic source".
 */
QString GenericSource::defaultName()
{
    const QString defName(QStringLiteral("Generic source"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

void GenericSource::setPhotonCountInSystem(SimpleCTsystem* system, double photonsPerPixel)
{
    if(system->source()->type() != GenericSource::Type)
        throw std::runtime_error("GenericSource::setPhotonCountInSystem(): Cannot set photon count,"
                                 " system does not contain a GenericSource.");

    auto fluxAdjustFactor = photonsPerPixel / system->photonsPerPixelMean();
    auto srcPtr = static_cast<GenericSource*>(system->source());
    srcPtr->setPhotonFlux(srcPtr->photonFlux() * fluxAdjustFactor);
}

/*!
 * Returns a formatted string with information about the object.
 */
QString GenericSource::info() const
{
    QString ret(AbstractSource::info());

    ret += typeInfoString(typeid(this));

    ret += (this->type() == GenericSource::Type) ? "}\n" : "";

    return ret;
}

// Use SerializationInterface::fromVariant() documentation.
void GenericSource::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    auto varMap = variant.toMap();
    _totalFlux = varMap.value("photon flux").toDouble();
    auto energy = varMap.value("energy range").toList();
    if(energy.size() != 2)
        qFatal("GenericSource::fromVariant(): Invalid number of values for energy range!");
    _energyRange.start() = energy.at(0).toFloat();
    _energyRange.end() = energy.at(1).toFloat();
    _samplingHint = varMap.value("sampling hint").toUInt();
}

// Use SerializationInterface::toVariant() documentation.
QVariant GenericSource::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("photon flux", _totalFlux);
    QVariantList energy;
    energy.append(_energyRange.start());
    energy.append(_energyRange.end());
    ret.insert("energy range", energy);
    ret.insert("sampling hint", _samplingHint);

    return ret;
}

/*!
 * \fn IntervalDataSeries GenericSource::spectrum(uint nbSamples) const
 *
 * Returns the emitted radiation spectrum sampled with \a nbSamples bins covering the energy
 * range of [energyRange().from, energyRange().to] keV. Using the output of
 * spectrumDiscretizationHint() as input for \a nbSamples results in a returned spectrum that
 * is identical to the one that has previously been set using setSpectrum() (unless the energy
 * range has been changed explicitely after setting the spectrum).
 *
 * \sa AbstractSource::spectrum(), setSpectrum(), setEnergyRange().
 */

/*!
 * Sets the energy range to \a range.
 *
 * Note that an appropriate energy range will be set automatically when using setSpectrum().
 * Use this method only if you specifically intend to change the energy range and yor are sure
 * that meaningful information for \a range is available from the spectrum set to this instance.
 */
void GenericSource::setEnergyRange(const EnergyRange& range) { _energyRange = range; }

/*!
 * Sets the spectrum of this instance to the sampled data provided by \a spectrum. Also sets the
 * energy range of this instance to the range covered by \a spectrum and stores the number of
 * samples in \a spectrum for later use in spectrumDiscretizationHint().
 *
 * Internally, a TabulatedDataModel is created that stores the data passed by \a spectrum. This
 * model is then used as this component's spectral model. Hence, calling
 * spectrum(spectrumDiscretizationHint()) after setting a spectrum using this method will return
 * the same data series as has been set.
 *
 * If \a updateFlux = \c true, the total flux will be set to the integral over the samples from
 * \a spectrum. Otherwise, the total flux remains unchanged. Use this only if you provide
 * unnormalized spectral data that encodes the total flux by its scaling.
 */
void GenericSource::setSpectrum(const IntervalDataSeries& spectrum, bool updateFlux)
{
    if(spectrum.nbSamples() == 0)
        throw std::runtime_error("GenericSource::setSpectrum(): Spectrum has no samples.");

    auto specModel = new FixedXraySpectrumModel;

    QMap<float, float> dataMap;
    for(uint smp = 0; smp < spectrum.nbSamples(); ++smp)
    {
        const auto& point = spectrum.data().at(smp);

        float binStart = point.x() - 0.5f * spectrum.binWidth();
        float binEnd = point.x() + 0.5f * spectrum.binWidth();

        // shift start of bin to next larger float
        if(smp > 0) // not the first bin
            binStart = std::nextafterf(binStart, point.x());

        dataMap.insert(binStart, point.y());
        dataMap.insert(binEnd,   point.y());
    }

    TabulatedDataModel spectrumData;
    spectrumData.setData(std::move(dataMap));

    specModel->setLookupTable(std::move(spectrumData));

    setSpectrumModel(specModel);
    _energyRange.start() = spectrum.samplingPoints().front() - 0.5f * spectrum.binWidth();
    _energyRange.end() = spectrum.samplingPoints().back() + 0.5f * spectrum.binWidth();
    _samplingHint = spectrum.nbSamples();

    if(updateFlux)
        _totalFlux = spectrum.integral();
}

// use documentation of base class
SystemComponent* GenericSource::clone() const { return new GenericSource(*this); }

/*!
 * Returns the energy range [in keV] of the radiation emitted by this instance.
 */
EnergyRange GenericSource::nominalEnergyRange() const { return _energyRange; }

/*!
 * Returns a hint for a reasonable number of sampling points when querying a spectrum of the
 * component. This returns the number of samples of the last spectrum set using setSpectrum().
 */
uint GenericSource::spectrumDiscretizationHint() const { return _samplingHint; }

/*!
 * Returns the nominal photon flux (photons/cm² in 1m distance).
 */
double GenericSource::nominalPhotonFlux() const { return _totalFlux; }

/*!
 * Sets total photon flux to \a flux (in photons/cm² in 1m distance).
 */
void GenericSource::setPhotonFlux(double flux) { _totalFlux = flux; }

} // namespace CTL
