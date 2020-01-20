#include "radiationencoder.h"
#include "components/abstractbeammodifier.h"
#include "components/abstractdetector.h"
#include "geometryencoder.h"
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

    foreach(const auto& modifier, _system->modifiers())
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

    foreach(const auto& modifier, _system->modifiers())
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

    foreach(const auto& modifier, _system->modifiers())
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
        const auto detResp = XYDataSeries::sampledFromModel(*_system->detector()->spectralResponseModel(),
                                                            spec.samplingPoints());
        for(uint smpl = 0; smpl < nbSamples; ++smpl)
            ret += spec.value(smpl) * spec.samplingPoint(smpl) * detResp.value(smpl);
    }
    else // regular mean energy
    {
        for(uint smpl = 0; smpl < nbSamples; ++smpl)
            ret += spec.value(smpl) * spec.samplingPoint(smpl);
    }

    return ret;
}

} // namespace CTL
