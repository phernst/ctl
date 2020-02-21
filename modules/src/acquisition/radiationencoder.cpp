#include "radiationencoder.h"
#include "components/abstractbeammodifier.h"
#include "components/abstractdetector.h"
#include "geometryencoder.h"
#include "models/stepfunctionmodels.h"
#include "models/xydataseries.h"

namespace CTL {

RadiationEncoder::RadiationEncoder(const SimpleCTsystem* system)
    : _system(system)
{
}

/*!
 * Returns the final radiation spectrum of the system. This considers the original spectrum emitted
 * by the source component as well as all modifications caused by beam modifiers (e.g. filters).
 *
 * The spectrum will by samples with \a nbSamples points over the interval defined by the
 * energyRange() method of the source component.
 */
IntervalDataSeries RadiationEncoder::finalSpectrum(uint nbSamples) const
{
    auto spectrum = _system->source()->spectrum(nbSamples);

    for(const auto& modifier : _system->modifiers())
        spectrum = modifier->modifiedSpectrum(spectrum);

    return spectrum;
}

/*!
 * Returns the final radiation spectrum of the system. This considers the original spectrum emitted
 * by the source component as well as all modifications caused by beam modifiers (e.g. filters).
 *
 * The spectrum will by samples with \a nbSamples points equally distributed over the interval
 * specified by \a range.
 */
IntervalDataSeries RadiationEncoder::finalSpectrum(EnergyRange range, uint nbSamples) const
{
    auto spectrum = _system->source()->spectrum(range, nbSamples);

    for(const auto& modifier : _system->modifiers())
        spectrum = modifier->modifiedSpectrum(spectrum);

    return spectrum;
}

/*!
 * Returns the final photon flux (i.e. photons per cm² in 1m distance) of the system. This considers
 * all properties of the source component as well as all modifications caused by beam modifiers
 * (e.g. filters).
 */
double RadiationEncoder::finalPhotonFlux() const
{
    auto spectrum = _system->source()->spectrum(_system->source()->spectrumDiscretizationHint());
    auto flux = _system->source()->photonFlux();

    for(const auto& modifier : _system->modifiers())
    {
        flux = modifier->modifiedFlux(flux, spectrum);
        spectrum = modifier->modifiedSpectrum(spectrum);
    }

    return flux;
}

/*!
 * Returns the number of photons that incide on a detector pixel averaged over all detector
 * modules.
 */
float RadiationEncoder::photonsPerPixelMean() const
{
    const auto& counts = photonsPerPixel();
    return std::accumulate(counts.begin(), counts.end(), 0.0f) / counts.size();
}

/*!
 * Returns the average number of photons that incide on a detector pixel in module \a module.
 */
float RadiationEncoder::photonsPerPixel(uint module) const
{
    constexpr auto convertUnit = 1.0e-2f; // convert unit of flux: 1/cm² -> 1/mm²
    return float(finalPhotonFlux()) * convertUnit *
           GeometryEncoder::effectivePixelArea(*_system, module);
}

/*!
 * Returns the average numbers of photons that incide on a detector pixel for all modules.
 */
std::vector<float> RadiationEncoder::photonsPerPixel() const
{
    constexpr auto convertUnit = 1.0e-2f; // convert unit of flux: 1/cm² -> 1/mm²
    const auto flux = float(finalPhotonFlux()) * convertUnit;

    const auto nbMod = _system->detector()->nbDetectorModules();
    std::vector<float> ret(nbMod);
    for(uint mod = 0; mod < nbMod; ++mod)
        ret[mod] = flux * GeometryEncoder::effectivePixelArea(*_system, mod);

    return ret;
}

/*!
 * Returns the average detective quantum efficiency of the detector. This value represents the
 * fraction of incoming photons (distributed w.r.t. to the incident radiation spectrum) that is
 * detected by the detector, considering the spectral response model of the detector.
 *
 * Thus, the number of detected photons per pixel (averaged over modules) would compute as:
 * photonsPerPixelMean() * detectiveQuantumEfficieny().
 */
float RadiationEncoder::detectiveQuantumEfficieny() const
{
    if(!_system->detector()->hasSpectralResponseModel())
        return 1.0f;

    const auto nbSamples = _system->source()->spectrumDiscretizationHint();
    const auto spec = finalSpectrum(nbSamples);
    const auto detResp = XYDataSeries::sampledFromModel(*_system->detector()->spectralResponseModel(),
                                                        spec.samplingPoints());

    return spec.integral(detResp.values());
}

/*!
 * Returns the average energy of a photon detected by the detector with respect to the incident
 * radiation spectrum and the spectral response model of the detector.
 */
float RadiationEncoder::detectiveMeanEnergy() const
{
    const auto nbSamples = _system->source()->spectrumDiscretizationHint();
    const auto spec = finalSpectrum(nbSamples);

    float ret = 0.0f;

    if(_system->detector()->hasSpectralResponseModel())
    {
        const auto detResp = XYDataSeries::sampledFromModel(
                                               *_system->detector()->spectralResponseModel(),
                                               spec.samplingPoints());
        for(auto smpl = 0u; smpl < nbSamples; ++smpl)
            ret += spec.value(smpl) * spec.samplingPoint(smpl) * detResp.value(smpl);
    }
    else // regular mean energy
    {
        for(auto smpl = 0u; smpl < nbSamples; ++smpl)
            ret += spec.value(smpl) * spec.samplingPoint(smpl);
    }

    return ret;
}

const SimpleCTsystem* RadiationEncoder::system() const
{
    return _system;
}

SpectralInformation RadiationEncoder::spectralInformation(AcquisitionSetup setup,
                                                          float energyResolution)
{
    if(energyResolution < 0.0f)
        throw std::runtime_error("RadiationEncoder::spectralInformation():"
                                 " Requested negative energy resolution!");

    SpectralInformation ret;

    const auto nbViews = setup.nbViews();
    const auto srcPtr = setup.system()->source();

    // find highest resolution and determine energy interval covering spectra of all views
    for(auto view = 0u; view < nbViews; ++view)
    {
        setup.prepareView(view);
        const auto viewEnergyRange = srcPtr->energyRange();
        const auto viewReso = viewEnergyRange.width() / float(srcPtr->spectrumDiscretizationHint());
        ret._bestReso = std::min(ret._bestReso, viewReso);
        ret._fullCoverage.start() = std::min(ret._fullCoverage.start(), viewEnergyRange.start());
        ret._fullCoverage.end()   = std::max(ret._fullCoverage.end(), viewEnergyRange.end());
    }

    qDebug() << "highestResolution: " << ret._bestReso;
    qDebug() << "fullCoverageInterval: [" << ret._fullCoverage.start() << " , "
                                          << ret._fullCoverage.end() << "]";

    ret._binWidth = energyResolution;
    if(qFuzzyIsNull(ret._binWidth)) // energy resolution is unset --> use highest resolution found in all views
        ret._binWidth = std::max(ret._bestReso, 0.1f); // minimum (automatic) bin width: 0.1 keV

    // set required number of samples (minimum of one sample) and update coverage interval
    uint nbEnergyBins = std::max({ uint(std::ceil(ret._fullCoverage.width() / ret._binWidth)), 1u });
    ret._fullCoverage.end() = ret._fullCoverage.start() + nbEnergyBins * ret._binWidth;

    ret.reserveMemory(nbEnergyBins, nbViews); // reserve memory

    RadiationEncoder radiationEnc(setup.system());

    // get (view-dependent) spectra
    for(auto view = 0u; view < nbViews; ++view)
    {
        setup.prepareView(view);
        ret.extractViewSpectrum(&radiationEnc, view);
    }

    return ret;
}

uint SpectralInformation::nbEnergyBins() const
{
    return static_cast<uint>(_bins.size());
}

float SpectralInformation::binWidth() const
{
    return _binWidth;
}

const SpectralInformation::BinInformation& SpectralInformation::bin(uint binIdx) const
{
    return _bins[binIdx];
}

const std::vector<double>& SpectralInformation::totalIntensity() const
{
    return _totalIntensities;
}

const Range<float>& SpectralInformation::fullCoverageRange() const
{
    return _fullCoverage;
}

float SpectralInformation::highestReso() const
{
    return _bestReso;
}

void SpectralInformation::reserveMemory(uint nbBins, uint nbViews)
{
    BinInformation defaultBin;
    defaultBin.intensities.resize(nbViews);
    defaultBin.adjustedFluxMods.resize(nbViews);

    _bins = std::vector<BinInformation>(nbBins, defaultBin);
    _totalIntensities = std::vector<double>(nbViews, 0.0);
}

void SpectralInformation::extractViewSpectrum(const RadiationEncoder* encoder, uint viewIdx)
{
    static const auto constModel = makeDataModel<ConstantModel>();
    const auto system = encoder->system();

    const IntervalDataSeries spectrum = encoder->finalSpectrum(_fullCoverage, nbEnergyBins());
    const auto globalFluxMod = system->source()->fluxModifier();
    const auto spectralResponse = system->detector()->hasSpectralResponseModel()
                                  ? system->detector()->spectralResponseModel()
                                  : constModel.get();

    for(auto bin = 0u, nbBins = nbEnergyBins(); bin < nbBins; ++bin)
    {
        const auto E = spectrum.samplingPoint(bin);
        _bins[bin].intensities[viewIdx] = spectrum.value(bin) * E;
        _bins[bin].adjustedFluxMods[viewIdx] = globalFluxMod * spectrum.value(bin) * spectralResponse->valueAt(E);
        _bins[bin].energy = E;
        _totalIntensities[viewIdx] += _bins[bin].intensities[viewIdx] * spectralResponse->valueAt(E);
    }

    _binWidth = spectrum.binWidth();
}

} // namespace CTL
