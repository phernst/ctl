#ifndef CTL_POISSONNOISEEXTENSION_H
#define CTL_POISSONNOISEEXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"

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
 *
 * The following code example shows how to extend a simple ray caster algorithm to add Poisson noise
 * to the simulated projections:
 * \code
 * // define volume and acquisition setup (incl. system)
 * auto volume = VolumeData::cube(100, 1.0f, 0.02f);
 * auto system = SimpleCTSystem::fromCTsystem(CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT(DetectorBinning::Binning4x4)));
 *
 * // reduce default radiation output of the source component (-> make noise more prominent)
 * static_cast<XrayTube*>(system.source())->setMilliampereSeconds(0.001);
 *
 * AcquisitionSetup acquisitionSetup(system, 10);
 * acquisitionSetup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0));
 *
 * // Core part
 * auto simpleProjector = new RayCasterProjector; // our simple projector
 *     // optional parameter settings for the projector
 *     // e.g. simpleProjector->settings().raySampling = 0.1f;
 *
 * // this is what you do without extension (i.e. noise-free):
 *     // simpleProjector->configure(acquisitionSetup);
 *     // ProjectionData projections = simpleProjector->project(volume);
 *
 * // instead we now do the following
 * PoissonNoiseExtension* extension = new PoissonNoiseExtension;
 *
 * extension->use(simpleProjector);                            // tell the extension to use the ray caster
 * extension->setFixedSeed(42);                                // setting a fixed seed for random number generation
 * extension->configure(acquisitionSetup);                     // configure the simulation
 *
 * ProjectionData projections = extension->project(volume);    // (compute and) get the final (noisy) projections
 *
 * // visualization (optional, requires 'ctl_qtgui' submodule):
 *     // gui::ProjectionViewer::plot(projections);
 * \endcode
 *
 * The difference between the projections with and without noise in the example above is shown here:
 * ![Simulated projections of a homogeneous water cube without (left) and with Poisson noise (right).](PoissonNoiseExtension.png)
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
    static void processViewCompact(SingleViewData& view, const std::vector<float>& i_0, uint seed);

    std::mt19937 _rng;
    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    bool _useParallelization{ true };
    bool _useFixedSeed{ false };
    uint _seed{ 0u };
};

} // namespace CTL

#endif // CTL_POISSONNOISEEXTENSION_H
