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

    void addNoiseToData(Chunk2D<double>& data, uint seed);
    void processView(SingleViewData& view, double i_0, uint seed);
    Chunk2D<double> transformedToCounts(const SingleViewData::ModuleData& module, double i_0) const;
    SingleViewData::ModuleData transformedToExtinctions(const Chunk2D<double>& counts,
                                                        double i_0) const;

    // for parallel computation
    static void processViewCompact(SingleViewData* view, double i_0, uint seed);
};

} // namespace CTL

#endif // POISSONNOISEEXTENSION_H
