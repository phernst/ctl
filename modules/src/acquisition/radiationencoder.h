#ifndef RADIATIONENCODER_H
#define RADIATIONENCODER_H

#include "acquisitionsetup.h"
#include "models/intervaldataseries.h"
#include "components/abstractsource.h"

namespace CTL {

class RadiationEncoder
{
public:
    RadiationEncoder(const SimpleCTsystem* system);

    IntervalDataSeries finalSpectrum(uint nbSamples) const;
    IntervalDataSeries finalSpectrum(EnergyRange range, uint nbSamples) const;
    double finalPhotonFlux() const;

    float photonsPerPixelMean() const;
    float photonsPerPixel(uint module) const;
    std::vector<float> photonsPerPixel() const;

    float detectiveQuantumEfficieny() const;
    float detectiveMeanEnergy() const;

private:
    // member variables
    const SimpleCTsystem* _system; //!< Pointer to system whose radiation shall be encoded.
};

} // namespace CTL

#endif // RADIATIONENCODER_H
