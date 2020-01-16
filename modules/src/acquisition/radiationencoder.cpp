#include "radiationencoder.h"
#include "components/abstractbeammodifier.h"
#include "components/abstractdetector.h"
#include "geometryencoder.h"

namespace CTL {

RadiationEncoder::RadiationEncoder(const SimpleCTsystem* system)
    : _system(system)
{

}

IntervalDataSeries RadiationEncoder::finalSpectrum(uint nbSamples) const
{
    auto spectrum = _system->source()->spectrum(nbSamples);

    foreach(const auto& modifier, _system->modifiers())
        spectrum = modifier->modifiedSpectrum(spectrum);

    return spectrum;
}

IntervalDataSeries RadiationEncoder::finalSpectrum(AbstractSource::EnergyRange range, uint nbSamples) const
{
    auto spectrum = _system->source()->spectrum(range, nbSamples);

    foreach(const auto& modifier, _system->modifiers())
        spectrum = modifier->modifiedSpectrum(spectrum);

    return spectrum;
}

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
    auto nbMod = _system->detector()->nbDetectorModules();
    std::vector<float> ret(nbMod);
    for(uint mod = 0; mod < nbMod; ++mod)
        ret[mod] = photonsPerPixel(mod);

    return ret;
}

} // namespace CTL
