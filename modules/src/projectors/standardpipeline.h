#ifndef STANDARDPIPELINE_H
#define STANDARDPIPELINE_H

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
 * - enableSpectralEffects()    - full spectral simulation (energy dependent attenuation and response) [enabled]
 *
 * Specific settings for all effects can be adjusted using the corresponding setter objects.
 *
 * The StandardPipeline supports two different options with respect to the order in which the
 * effects are processed internally:
 * - SimulationPolicy::Fast
 * - SimulationPolicy::Accurate
 *
 * The Fast setting (default) uses the approximation of processing Poisson noise after the spectral
 * effects. This leads to substantial acceleration with slight loss in accuracy.
 * However, in case a spectral detector response is in use, the use of the Fast setting is strongly
 * discouraged, because it then leads to incorrect results.
 * In the Accurate setting, Poisson noise is processed for each individual energy bin requested by
 * the spectral effects extension. While being most accurate, this option is substatially more
 * time-consuming and not strongly required in many situations.
 * The approximation behavior must be decided in the constructor and cannot be changed afterwards.
 *
 * StandardPipeline uses OCL::RayCasterProjector as the actual forward projector. Its settings can
 * be adjusted calling the corresponding member methods of settingsRayCaster().
 *
 * A fully-enabled pipeline is composed as follows:
 *
 * <i>Volume Data</i> <- OCL::RayCasterProjector <- ArealFocalSpotExtension <- SpectralEffectsExtension <- PoissonNoiseExtension <- DetectorSaturationExtension  [Fast]<br>
 * <i>Volume Data</i> <- OCL::RayCasterProjector <- ArealFocalSpotExtension <- PoissonNoiseExtension <- SpectralEffectsExtension <- DetectorSaturationExtension  [Accurate]
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
 * auto volume = SpectralVolumeData::createBall(50.0f, 0.5f, 1.0f,
 *                                              database::attenuationModel(database::Composite::Water));
 *
 * // create a C-arm CT system and a short scan protocol with 10 views
 * auto system = SimpleCTsystem::fromCTsystem(CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT()));
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
    enum SimulationPolicy
    {
        Fast,
        Accurate,
    };

    StandardPipeline(SimulationPolicy policy = StandardPipeline::Fast);
    ~StandardPipeline();

    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;
    ProjectionData projectComposite(const CompositeVolume &volume) override;
    bool isLinear() const override;

    // SerializationInterface interface
    void fromVariant(const QVariant& variant) override;
    QVariant toVariant() const override;

    // configuration methods
    void enableArealFocalSpot(bool enable = true);
    void enableDetectorSaturation(bool enable = true);
    void enablePoissonNoise(bool enable = true);
    void enableSpectralEffects(bool enable = true);

//    void setArealFocalSpotDiscretization(const QSize& discretization);
//    void setDetectorSaturationSampling(float energyBinWidth);
//    void setSpectralEffectsSampling(float energyBinWidth);
//    void setPoissonFixedSeed(uint seed);
//    void setPoissonRandomSeedMode();
//    void setPoissonParallelizationMode(bool enabled);
//    void setRayCasterInterpolation(bool enabled);
//    void setRayCasterRaysPerPixel(const QSize& sampling);
//    void setRayCasterRaySampling(float sampling);
//    void setRayCasterVolumeUpSampling(uint upsamplingFactor);

    SettingsAFS settingsArealFocalSpot();
    SettingsDetectorSaturation settingsDetectorSaturation();
    SettingsPoissonNoise settingsPoissonNoise();
    SettingsSpectralEffects settingsSpectralEffects();
    SettingsRayCaster settingsRayCaster();

private:
    ProjectionPipeline _pipeline; //!< The pipeline object; owns the projector and all extensions.

    bool _arealFSEnabled = false;     //!< enabled/disabled state variable for areal focal spot
    bool _detSatEnabled = false;      //!< enabled/disabled state variable for detector saturation
    bool _spectralEffEnabled = false; //!< enabled/disabled state variable for spectral effects
    bool _poissonEnabled = false;     //!< enabled/disabled state variable for Poisson noise
    bool _approxMode = true;          //!< enabled/disabled state variable for approximation mode

    OCL::RayCasterProjector* _projector;     //!< Pointer to the ray caster projector.
    ArealFocalSpotExtension* _extAFS;        //!< Pointer to the ArealFocalSpotExtension.
    DetectorSaturationExtension* _extDetSat; //!< Pointer to the DetectorSaturationExtension.
    PoissonNoiseExtension* _extPoisson;      //!< Pointer to the PoissonNoiseExtension.
    SpectralEffectsExtension* _extSpectral;  //!< Pointer to the SpectralEffectsExtension.

    uint posAFS() const;
    uint posDetSat() const;
    uint posPoisson() const;
    uint posSpectral() const;

    class SettingsAFS
    {
    public:
        void setDiscretization(const QSize& discretization);
        void enableLowExtinctionApproximation(bool enable = true);

        SettingsAFS(const SettingsAFS&) = delete;
        SettingsAFS& operator=(const SettingsAFS&) = delete;
    private:
        SettingsAFS(ArealFocalSpotExtension* ext) : _ext(ext) {}
        ArealFocalSpotExtension* _ext;

        friend class StandardPipeline;
    };

    class SettingsDetectorSaturation
    {
    public:
        void setSpectralSamples(uint nbSamples);

        SettingsDetectorSaturation(const SettingsDetectorSaturation&) = delete;
        SettingsDetectorSaturation& operator=(const SettingsDetectorSaturation&) = delete;
    private:
        SettingsDetectorSaturation(DetectorSaturationExtension* ext) : _ext(ext) {}
        DetectorSaturationExtension* _ext;

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
        SettingsPoissonNoise(PoissonNoiseExtension* ext) : _ext(ext) {}
        PoissonNoiseExtension* _ext;

        friend class StandardPipeline;
    };

    class SettingsSpectralEffects
    {
    public:
        void setSamplingResolution(float energyBinWidth);

        SettingsSpectralEffects(const SettingsSpectralEffects&) = delete;
        SettingsSpectralEffects& operator=(const SettingsSpectralEffects&) = delete;
    private:
        SettingsSpectralEffects(SpectralEffectsExtension* ext) : _ext(ext) {}
        SpectralEffectsExtension* _ext;

        friend class StandardPipeline;
    };

    class SettingsRayCaster
    {
    public:
        void setInterpolation(bool enabled);
        void setRaysPerPixel(const QSize& sampling);
        void setRaySampling(float sampling);
        void setVolumeUpSampling(uint upsamplingFactor);

        SettingsRayCaster(const SettingsRayCaster&) = delete;
        SettingsRayCaster& operator=(const SettingsRayCaster&) = delete;
    private:
        SettingsRayCaster(OCL::RayCasterProjector* proj) : _proj(proj) {}
        OCL::RayCasterProjector* _proj;

        friend class StandardPipeline;
    };
};

}

/*! \file */
///@{
///@}

#endif // STANDARDPIPELINE_H
