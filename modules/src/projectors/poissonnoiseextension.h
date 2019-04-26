#ifndef POISSONNOISEEXTENSION_H
#define POISSONNOISEEXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"
#include "projectors/abstractprojectorconfig.h"

#include <random>

namespace CTL {

class PoissonNoiseExtension : public ProjectorExtension
{
public:
    using ProjectorExtension::ProjectorExtension;

    // ProjectorExtension interface
    void configure(const AcquisitionSetup& setup, const AbstractProjectorConfig& config) override;
    bool isLinear() const override;

    void setFixedSeed(uint seed);
    void setRandomSeedMode();
    void setParallelizationEnabled(bool enabled);

protected:
    ProjectionData extendedProject(const MetaProjector& nestedProjector) override;

private:
    bool _useParallelization = true;
    bool _useFixedSeed = false;
    std::mt19937 _rng;

    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    std::unique_ptr<AbstractProjectorConfig> _config; //!< A copy of the projector configuration.

    static void processViewCompact(SingleViewData* view, std::vector<float> i_0, uint seed);
};

} // namespace CTL

#endif // POISSONNOISEEXTENSION_H
