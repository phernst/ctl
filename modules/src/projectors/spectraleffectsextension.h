#ifndef SPECTRALEFFECTSEXTENSION_H
#define SPECTRALEFFECTSEXTENSION_H

#include "projectorextension.h"
#include "abstractprojectorconfig.h"
#include "acquisition/acquisitionsetup.h"

namespace CTL {

class SpectralEffectsExtension : public ProjectorExtension
{
public:
    using ProjectorExtension::ProjectorExtension;
    SpectralEffectsExtension() = default;
    explicit SpectralEffectsExtension(float energyBinWidth);

    void configure(const AcquisitionSetup& setup, const AbstractProjectorConfig& config) override;
    ProjectionData project(const VolumeData& volume) override;

    ProjectionData projectComposite(const CompositeVolume& volume) override;
    bool isLinear() const override;

    void setSpectralSamplingResolution(float energyBinWidth);

private:
    struct SpectralInformation
    {
        std::vector<std::vector<double>> intensities;
        std::vector<std::vector<double>> adjustedFluxMods;
        std::vector<double> totalIntensities;
        std::vector<float> energyBins;
        float binWidth{};
        uint nbSamples{};
    };

    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    std::unique_ptr<AbstractProjectorConfig> _config; //!< A copy of the projector configuration.
    SpectralInformation _spectralInfo;
    float _deltaE = 0.0f;   

    void updateSpectralInformation();
    void applyDetectorResponse(ProjectionData& intensity, float energy) const;
};


} // namespace CTL

#endif // SPECTRALEFFECTSEXTENSION_H
