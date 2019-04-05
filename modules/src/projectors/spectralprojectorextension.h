#ifndef SPECTRALPROJECTOREXTENSION_H
#define SPECTRALPROJECTOREXTENSION_H

#include "projectorextension.h"
#include "abstractprojectorconfig.h"
#include "acquisition/acquisitionsetup.h"

namespace CTL {

class SpectralProjectorExtension : public ProjectorExtension
{
public:
    using ProjectorExtension::ProjectorExtension;
    SpectralProjectorExtension(uint nbSamples);

    void configure(const AcquisitionSetup& setup, const AbstractProjectorConfig& config) override;
    ProjectionData project(const VolumeData& volume) override;

    ProjectionData projectComposite(const CompositeVolume& volume) override;

    void setSpectralRange(float from, float to);
    void setSpectralSampling(uint nbSamples);

private:
    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    std::unique_ptr<AbstractProjectorConfig> _config; //!< A copy of the projector configuration.

    float _from = 0.0f;
    float _to   = -1.0f;
    uint _nbSamples = 10;

};


} // namespace CTL

#endif // SPECTRALPROJECTOREXTENSION_H
