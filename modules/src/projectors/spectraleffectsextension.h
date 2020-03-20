#ifndef CTL_SPECTRALEFFECTSEXTENSION_H
#define CTL_SPECTRALEFFECTSEXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"
#include "processing/coordinates.h" // Range<T>
#include "acquisition/radiationencoder.h"

namespace CTL {

/*!
 * \class SpectralEffectsExtension
 *
 * \brief The SpectralEffectsExtension class is an extension for forward projectors that provides
 * functionality to consider spectral effects.
 *
 * This extension adds the functionality to consider spectral effects within the projections. These
 * effects encompass:
 * - spectrally-dependent radiation output (polychromatic spectrum)
 * - spectrally-dependent attenuation coefficients
 * - (spectral) detector response function/model
 *
 * Generally speaking, this extension sub-divides the projection task into multiple subtasks, each
 * representing a single energy bin, for within which monoenergetic behavior (corresponding to the
 * bin energy) is assumed. Depending on whether the nested projector routine is linear or not, the
 * actual performed procedure differs. More detailed information is provided in project() and
 * projectComposite().
 *
 * Meaningful results are achieved only if this extension is used in combination with volume data
 * that contains full spectral information (see SpectralVolumeData::hasSpectralInformation()). In
 * case volume data without spectral information is used, this extension will be bypassed (Note that
 * the combination of non-spectral volume data with a detector component that has explicitely being
 * assigned a spectral response model is contradictory, and consequently an exception will be
 * thrown).
 *
 * \code
 *     // define volume as a ball filled with attenuation 0.081/mm (approx. bone @ 50 keV)
 *     auto volume      = VoxelVolume<float>::ball(50.0f, 0.5f, 0.081f);
 *     // create a spectral volume using the voxel data from volume and the correct attenuation model (for bone)
 *     auto spectralVol = SpectralVolumeData::fromMuVolume(volume,
 *                                                         database::attenuationModel(database::Composite::Bone_Cortical));
 *
 *     auto system = SimpleCTsystem::fromCTsystem(CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT(DetectorBinning::Binning4x4)));
 *     AcquisitionSetup acquisitionSetup(system, 10);
 *     acquisitionSetup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0));
 *
 *     // Core part
 *     auto simpleProjector = new RayCasterProjector; // our simple projector
 *         // optional parameter settings for the projector
 *         // e.g. simpleProjector->settings().raySampling = 0.1f;
 *
 *     // this is what you do without extension:
 *         // simpleProjector->configure(acquisitionSetup);
 *         // ProjectionData projections = simpleProjector->project(volume);
 *         // Note that we used the 'plain' volume here; instead we could also do:
 *         // ProjectionData projections = simpleProjector->project(*spectralVol.muVolume(50.0f));
 *
 *     // To consider spectral effects, we now do the following:
 *     SpectralEffectsExtension* extension = new SpectralEffectsExtension;
 *
 *     extension->use(simpleProjector);                                // tell the extension to use the ray caster
 *     extension->setSpectralSamplingResolution(10.0f);                // set the energy resolution for spectral effects
 *     extension->configure(acquisitionSetup);                         // configure the simulation
 *
 *     ProjectionData projections = extension->project(spectralVol);   // (compute and) get the final projections
 *     // Note that we must use a 'spectral' volume here for spectral effects to be considered.
 *     // (Passing the 'plain' volume leads to the same result as using the ray caster without extension.)
 * \endcode
 *
 * The difference between projections with and without spectral effects in the example above is shown here:
 * ![Simulated projections of a homogeneous bone ball without (left) and with spectral effects (right) considered.](SpectralEffectsExtension.png)
 *
 * The following plot shows a comparison of the normalized absorption profiles shown in the figures above.
 * ![Normalized absorption profiles without (red) and with spectral effects (black) considered.](SpectralEffectsExtension_comparison.png)
 */
class SpectralEffectsExtension : public ProjectorExtension
{
    CTL_TYPE_ID(104)

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;
    public: ProjectionData project(const VolumeData& volume) override;

public:
    SpectralEffectsExtension() = default;
    explicit SpectralEffectsExtension(float energyBinWidth);
    using ProjectorExtension::ProjectorExtension;

    ProjectionData projectComposite(const CompositeVolume& volume) override;
    bool isLinear() const override;

    // SerializationInterface interface
    QVariant toVariant() const override;
    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

    void setSpectralSamplingResolution(float energyBinWidth);

private:  
    void updateSpectralInformation();
    bool canBypassExtension(const CompositeVolume& volume) const;
    void applyDetectorResponse(ProjectionData& intensity, float energy) const;

    using BinInformation = SpectralInformation::BinInformation;

    ProjectionData projectLinear(const CompositeVolume& volume);
    ProjectionData projectNonLinear(const CompositeVolume& volume);
    ProjectionData singleBinIntensityLinear(const std::vector<ProjectionData>& materialProjs,
                                            const std::vector<float>& massAttenCoeffs,
                                            const BinInformation& binInfo);
    ProjectionData singleBinIntensityNonLinear(const CompositeVolume& volume,
                                               const BinInformation& binInfo);

    void addDummyPrepareSteps();
    void removeDummyPrepareSteps();
    void replaceDummyPrepareSteps(const BinInformation& binInfo, float binWidth);

    SpectralInformation _spectralInfo;
    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    float _deltaE{ 0.0f };
};


} // namespace CTL

#endif // CTL_SPECTRALEFFECTSEXTENSION_H
