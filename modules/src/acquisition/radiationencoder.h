#ifndef RADIATIONENCODER_H
#define RADIATIONENCODER_H

#include "acquisitionsetup.h"
#include "models/intervaldataseries.h"
#include "components/abstractsource.h"

namespace CTL {

class RadiationEncoder;

class SpectralInformation
{
public:
    struct BinInformation
    {
        std::vector<double> intensities;      // for each view
        std::vector<double> adjustedFluxMods; // for each view
        float energy;
    };

    uint nbEnergyBins() const;
    float binWidth() const;
    const BinInformation& bin(uint binIdx) const;
    const std::vector<double>& totalIntensity() const;
    const Range<float>& fullCoverageRange() const;
    float highestReso() const;

    void reserveMemory(uint nbBins, uint nbViews);

private:
    float _binWidth{};
    std::vector<BinInformation> _bins;       // for each bin
    std::vector<double> _totalIntensities;   // for each view
    Range<float> _fullCoverage = { std::numeric_limits<float>::max(), 0.0f };
    float _bestReso = std::numeric_limits<float>::max();

    void extractViewSpectrum(const RadiationEncoder* encoder, uint viewIdx);

    friend class RadiationEncoder;
};

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

    const SimpleCTsystem* system() const;

    static SpectralInformation spectralInformation(AcquisitionSetup setup, float energyResolution = 0.0f);

private:
    // member variables
    const SimpleCTsystem* _system; //!< Pointer to system whose radiation shall be encoded.
};

} // namespace CTL

#endif // RADIATIONENCODER_H
