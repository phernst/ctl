#ifndef CTL_STANDARDPIPELINE_H
#define CTL_STANDARDPIPELINE_H

#include "projectionpipeline.h"

namespace CTL {

namespace OCL {
    class RayCasterProjector;
}
class ArealFocalSpotExtension;
class DetectorSaturationExtension;
class PoissonNoiseExtension;
class SpectralEffectsExtension;


/*!
 * \class StandardPipeline
 *
 * \brief The StandardPipeline class is a convenience class to work with a predefined processing
 * pipeline for creation of projections.
 *
 * This class provides a preset arrangement of projector and extensions in a meaningful composition.
 * Individual simulation effects can be simply enabled/disabled using the corresponding methods
 * (default setting in brackets) :
 * - enableArealFocalSpot()     - simulation of finite focal spot size [disabled]
 * - enableDetectorSaturation() - simulation of over-/undersaturation effects [disabled]
 * - enablePoissonNoise()       - simulation of Poisson noise [enabled]
 * - enableSpectralEffects()    - full spectral simulation (energy dependent attenuation and
 * response) [enabled]
 *
 * Specific settings for all effects can be adjusted using the corresponding setter objects.
 *
 * The StandardPipeline supports three different options with respect to degree of approximation
 * used in the processing of individual effects:
 * - ApproximationPolicy::No_Approximation
 * - ApproximationPolicy::Default_Approximation
 * - ApproximationPolicy::Full_Approximation
 *
 * The Default_Approximation setting (default) uses the approximation of processing Poisson noise
 * after the spectral effects. This leads to substantial acceleration with slight loss in accuracy.
 * However, in case a spectral detector response is in use, the use of the Fast setting is strongly
 * discouraged, because it then leads to incorrect results.
 * In addition to the approximation described above, the Full_Approximation also uses the linearized
 * setting for the ArealFocalSpotExtension. This uses sub-sample averaging in extinction domain and
 * leads to further increases in computation speed, but yields inaccurate results in case of strong
 * extinction gradients (e.g. edges) in the  projection images.
 * In the No_Approximation setting (most accurate), Poisson noise is processed for each individual
 * energy bin requested by the spectral effects extension. While being most accurate, this option is
 * substatially more time-consuming and not strongly required in many situations.
 * The approximation behavior must be decided in the constructor and cannot be changed afterwards.
 *
 * StandardPipeline uses OCL::RayCasterProjector as the actual forward projector. Its settings can
 * be adjusted calling the corresponding member methods of settingsRayCaster().
 *
 * A fully-enabled pipeline is composed as follows:
 *
 * <i>Volume Data</i> <- OCL::RayCasterProjector <- ArealFocalSpotExtension <-
 * SpectralEffectsExtension <- PoissonNoiseExtension <- DetectorSaturationExtension
 * [Default_Approximation or Full_Approximation]<br>
 * <i>Volume Data</i> <- OCL::RayCasterProjector <- ArealFocalSpotExtension <- PoissonNoiseExtension
 *  <- SpectralEffectsExtension <- DetectorSaturationExtension  [No_Approximation]
 *
 * The StandardPipeline object itself can be used in the same way as any projector; use configure()
 * to pass the AcquisitionSetup for the simulation and then call project() (or projectComposite())
 * with the volume dataset that shall be projected to create the simulated projections using the
 * full processing pipeline that is managed your StandardPipeline object.
 *
 * The following code example demonstrates the usage of StandardPipeline for creating projections
 * of a water ball phantom. In addition to its standard settings, we want to also enable the
 * simulation of an areal focal spot (with default sampling of 3x3 sub-samples) and set the energy
 * resolution for spectral effects to 5 keV (i.e. bin width).
 * \code
 * // create a water ball
 * auto volume = SpectralVolumeData::ball(50.0f, 0.5f, 1.0f,
 *                                        database::attenuationModel(database::Composite::Water));
 *
 * // create a C-arm CT system and a short scan protocol with 10 views
 * auto system = CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * auto setup = AcquisitionSetup(system, 10);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0));
 *
 * // create the standard pipeline and adjust the desired settings (focal spot & energy resolution)
 * StandardPipeline pipe;
 * pipe.enableArealFocalSpot();
 * pipe.settingsSpectralEffects().setSamplingResolution(5.0f);
 *
 * // pass the acquisition setup and run the simulation
 * pipe.configure(setup);
 * auto projections = pipe.project(volume);
 * \endcode
 */
class StandardPipeline: public AbstractProjector
{
    CTL_TYPE_ID(201)

    class SettingsAFS;
    class SettingsDetectorSaturation;
    class SettingsPoissonNoise;
    class SettingsSpectralEffects;
    class SettingsRayCaster;

public:    
    enum ApproximationPolicy
    {
        No_Approximation,
        Default_Approximation,
        Full_Approximation,
    };

    StandardPipeline(ApproximationPolicy policy = StandardPipeline::Default_Approximation);
    ~StandardPipeline() override;

    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;
    ProjectionData projectComposite(const CompositeVolume &volume) override;
    bool isLinear() const override;
    ProjectorNotifier* notifier() override;

    // SerializationInterface interface
    void fromVariant(const QVariant& variant) override;
    QVariant toVariant() const override;

    // configuration methods
    void enableArealFocalSpot(bool enable = true);
    void enableDetectorSaturation(bool enable = true);
    void enablePoissonNoise(bool enable = true);
    void enableSpectralEffects(bool enable = true);

    SettingsAFS settingsArealFocalSpot();
    SettingsDetectorSaturation settingsDetectorSaturation();
    SettingsPoissonNoise settingsPoissonNoise();
    SettingsSpectralEffects settingsSpectralEffects();
    SettingsRayCaster settingsRayCaster();

private:
    class SettingsAFS
    {
    public:
        void setDiscretization(const QSize& discretization);
        void enableLowExtinctionApproximation(bool enable = true);

        SettingsAFS(const SettingsAFS&) = delete;
        SettingsAFS& operator=(const SettingsAFS&) = delete;
    private:
        SettingsAFS(ArealFocalSpotExtension& ext) : _ext(ext) {}
        SettingsAFS(ArealFocalSpotExtension&& ext) = delete;
        ArealFocalSpotExtension& _ext;

        friend class StandardPipeline;
    };

    class SettingsDetectorSaturation
    {
    public:
        void setSpectralSamples(uint nbSamples);

        SettingsDetectorSaturation(const SettingsDetectorSaturation&) = delete;
        SettingsDetectorSaturation& operator=(const SettingsDetectorSaturation&) = delete;
    private:
        SettingsDetectorSaturation(DetectorSaturationExtension& ext) : _ext(ext) {}
        SettingsDetectorSaturation(DetectorSaturationExtension&& ext) = delete;
        DetectorSaturationExtension& _ext;

        friend class StandardPipeline;
    };

    class SettingsPoissonNoise
    {
    public:
        void setFixedSeed(uint seed);
        void setRandomSeedMode();
        void setParallelizationMode(bool enabled);

        SettingsPoissonNoise(const SettingsPoissonNoise&) = delete;
        SettingsPoissonNoise& operator=(const SettingsPoissonNoise&) = delete;
    private:
        SettingsPoissonNoise(PoissonNoiseExtension& ext) : _ext(ext) {}
        SettingsPoissonNoise(PoissonNoiseExtension&& ext) = delete;
        PoissonNoiseExtension& _ext;

        friend class StandardPipeline;
    };

    class SettingsSpectralEffects
    {
    public:
        void setSamplingResolution(float energyBinWidth);

        SettingsSpectralEffects(const SettingsSpectralEffects&) = delete;
        SettingsSpectralEffects& operator=(const SettingsSpectralEffects&) = delete;
    private:
        SettingsSpectralEffects(SpectralEffectsExtension& ext) : _ext(ext) {}
        SettingsSpectralEffects(SpectralEffectsExtension&& ext) = delete;
        SpectralEffectsExtension& _ext;

        friend class StandardPipeline;
    };

    class SettingsRayCaster
    {
    public:
        void setInterpolation(bool enabled);
        void setRaysPerPixel(const QSize& sampling);
        void setRaySampling(float sampling);
        void setVolumeUpSampling(uint upSamplingFactor);

        SettingsRayCaster(const SettingsRayCaster&) = delete;
        SettingsRayCaster& operator=(const SettingsRayCaster&) = delete;
    private:
        SettingsRayCaster(OCL::RayCasterProjector& proj) : _proj(proj) {}
        SettingsRayCaster(OCL::RayCasterProjector&& proj) = delete;
        OCL::RayCasterProjector& _proj;

        friend class StandardPipeline;
    };

    uint posAFS() const;
    uint posDetSat() const;
    uint posPoisson() const;
    uint posSpectral() const;

    ProjectionPipeline _pipeline; //!< The pipeline object; owns the projector and all extensions.

    OCL::RayCasterProjector* _projector;     //!< Pointer to the ray caster projector.
    ArealFocalSpotExtension* _extAFS;        //!< Pointer to the ArealFocalSpotExtension.
    DetectorSaturationExtension* _extDetSat; //!< Pointer to the DetectorSaturationExtension.
    PoissonNoiseExtension* _extPoisson;      //!< Pointer to the PoissonNoiseExtension.
    SpectralEffectsExtension* _extSpectral;  //!< Pointer to the SpectralEffectsExtension.

    ApproximationPolicy _approxMode;  //!< approximation level for the simulation
    bool _arealFSEnabled = false;     //!< enabled/disabled state variable for areal focal spot
    bool _detSatEnabled = false;      //!< enabled/disabled state variable for detector saturation
    bool _spectralEffEnabled = false; //!< enabled/disabled state variable for spectral effects
    bool _poissonEnabled = false;     //!< enabled/disabled state variable for Poisson noise
};

} // namespace CTL

/*! \file */
///@{
///@}

#endif // CTL_STANDARDPIPELINE_H
