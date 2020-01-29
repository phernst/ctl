#ifndef POISSONNOISEEXTENSION_H
#define POISSONNOISEEXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"
#include "projectors/abstractprojectorconfig.h"

#include <random>

namespace CTL {

class PoissonNoiseExtension : public ProjectorExtension
{
    CTL_TYPE_ID(103)

public:
    using ProjectorExtension::ProjectorExtension;

    // ProjectorExtension interface
    void configure(const AcquisitionSetup& setup) override;
    bool isLinear() const override;

    // SerializationInterface interface
    void fromVariant(const QVariant &variant) override;
    QVariant toVariant() const override;

    void setFixedSeed(uint seed);
    void setRandomSeedMode();
    void setParallelizationEnabled(bool enabled);

protected:
    ProjectionData extendedProject(const MetaProjector& nestedProjector) override;

private:
    bool _useParallelization = true;
    bool _useFixedSeed = false;
    uint _seed = 0;
    std::mt19937 _rng;

    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.

    static void processViewCompact(SingleViewData& view, const std::vector<float>& i_0, uint seed);
};

} // namespace CTL

#endif // POISSONNOISEEXTENSION_H
