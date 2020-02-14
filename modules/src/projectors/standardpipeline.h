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

    using ProjectorPtr = std::unique_ptr<AbstractProjector>;
    using ExtensionPtr = std::unique_ptr<ProjectorExtension>;

    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;
    ProjectionData projectComposite(const CompositeVolume &volume) override;
    bool isLinear() const override;

    // SerializationInterface interface
    void fromVariant(const QVariant& variant) override;
    QVariant toVariant() const override;

    StandardPipeline(SimulationPolicy policy = StandardPipeline::Fast);
    ~StandardPipeline();

    // configuration methods
    void enableArealFocalSpot(bool enable = true);
    void enableDetectorSaturation(bool enable = true);
    void enablePoissonNoise(bool enable = true);
    void enableSpectralEffects(bool enable = true);
    void enableApproxmiationMode(bool enable = true);


    void setArealFocalSpotDiscretization(const QSize& discretization);
    void setDetectorSaturationSampling(float energyBinWidth);
    void setSpectralEffectsSampling(float energyBinWidth);
    void setPoissonFixedSeed(uint seed);
    void setPoissonRandomSeedMode();
    void setPoissonParallelizationMode(bool enabled);
    void setRayCasterInterpolation(bool enabled);
    void setRayCasterRaysPerPixel(const QSize& sampling);
    void setRayCasterRaySampling(float sampling);
    void setRayCasterVolumeUpSampling(uint upsamplingFactor);

    SettingsAFS settingsArealFocalSpot() { return { _extAFS }; }
    SettingsDetectorSaturation settingsDetectorSaturation() { return { _extDetSat }; }
    SettingsPoissonNoise settingsPoissonNoise() { return { _extPoisson }; }
    SettingsSpectralEffects settingsSpectralEffects() { return { _extSpectral }; }
    SettingsRayCaster settingsRayCaster() { return { _projector }; }

private:
    ProjectionPipeline _pipeline;

    bool _arealFSEnabled = false;
    bool _detSatEnabled = false;
    bool _spectralEffEnabled = false;
    bool _poissonEnabled = false;
    bool _approxMode = true;

    OCL::RayCasterProjector* _projector;
    ArealFocalSpotExtension* _extAFS;
    DetectorSaturationExtension* _extDetSat;
    PoissonNoiseExtension* _extPoisson;
    SpectralEffectsExtension* _extSpectral;

    uint posAFS() const;
    uint posDetSat() const;
    uint posPoisson() const;
    uint posSpectral() const;

    class SettingsAFS
    {
    public:
        void setDiscretization(const QSize& discretization);

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
        void setSamplingResolution(float energyBinWidth);

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
#endif // STANDARDPIPELINE_H
