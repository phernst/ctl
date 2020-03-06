#ifndef POISSONNOISEEXTENSION_H
#define POISSONNOISEEXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"
#include "projectors/abstractprojectorconfig.h"

#include <random>

namespace CTL {

/*!
 * \class PoissonNoiseExtension
 *
 * \brief The PoissonNoiseExtension class is an extension for forward projectors that adds
 * Poisson-distributed noise to the projection data.
 *
 * This extension performs a postprocessing on the projection data to add Poisson-distributed noise
 * to the projections. For counts larger than 1.0e4. the Poisson distribution is approximated by a
 * normal distribution.
 *
 * Poisson-distributed random numbers are generated using the Mersenne twister engine
 * (std::mt19937). A fixed seed can be used to create reproducible results (see setFixedSeed()).
 * \code
 *
 * \endcode
 */
class PoissonNoiseExtension : public ProjectorExtension
{
    CTL_TYPE_ID(103)

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;

public:
    PoissonNoiseExtension() = default;
    using ProjectorExtension::ProjectorExtension;
    explicit PoissonNoiseExtension(uint fixedSeed, bool useParalellization = true);

    // ProjectorExtension interface
    bool isLinear() const override;

    // SerializationInterface interface
    QVariant toVariant() const override;
    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

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
