#include "standardpipeline.h"

#include "raycasterprojector.h"
#include "arealfocalspotextension.h"
#include "detectorsaturationextension.h"
#include "poissonnoiseextension.h"
#include "spectraleffectsextension.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(StandardPipeline)

StandardPipeline::StandardPipeline(SimulationPolicy policy)
{
    _extAFS      = new ArealFocalSpotExtension;
    _extDetSat   = new DetectorSaturationExtension;
    _extPoisson  = new PoissonNoiseExtension;
    _extSpectral = new SpectralEffectsExtension;

    _projector   = new OCL::RayCasterProjector;
    _pipeline.setProjector(_projector);

    if(policy == SimulationPolicy::Accurate)
        _approxMode = false;
}

StandardPipeline::~StandardPipeline()
{
    enableArealFocalSpot(true);
    enableDetectorSaturation(true);
    enablePoissonNoise(true);
    enableSpectralEffects(true);
}

void StandardPipeline::enableArealFocalSpot(bool enable)
{
    if(enable == _arealFSEnabled) // no change
        return;

    if(enable) // insert AFS into pipeline (first position)
        _pipeline.insertExtension(posAFS(), _extAFS);
    else // remove AFS from pipeline (first position)
        _pipeline.releaseExtension(posAFS());

    _arealFSEnabled = enable;
}

void StandardPipeline::enableDetectorSaturation(bool enable)
{
    if(enable == _detSatEnabled) // no change
        return;

    if(enable) // insert det. sat. into pipeline (last position)
        _pipeline.appendExtension(_extDetSat);
    else // remove det. sat. from pipeline (last position)
        _pipeline.releaseExtension(_pipeline.nbExtensions() - 1u);

    _detSatEnabled = enable;
}

void StandardPipeline::enablePoissonNoise(bool enable)
{
    if(enable == _poissonEnabled) // no change
        return;

    if(enable) // insert Poisson into pipeline (after AFS and spectral)
        _pipeline.insertExtension(posPoisson(), _extPoisson);
    else // remove Poisson from pipeline (after AFS and spectral)
        _pipeline.releaseExtension(posPoisson());

    _poissonEnabled = enable;
}

void StandardPipeline::enableSpectralEffects(bool enable)
{
    if(enable == _spectralEffEnabled) // no change
        return;

    if(enable) // insert spectral ext. into pipeline (after AFS)
        _pipeline.insertExtension(posSpectral(), _extSpectral);
    else // remove Poisson from pipeline (after AFS)
        _pipeline.releaseExtension(posSpectral());

    _spectralEffEnabled = enable;
}

void StandardPipeline::enableApproxmiationMode(bool enable)
{
    _approxMode = enable;
}

void StandardPipeline::setArealFocalSpotDiscretization(const QSize& discretization)
{
    _extAFS->setDiscretization(discretization);
}

void StandardPipeline::setDetectorSaturationSampling(float energyBinWidth)
{
    _extDetSat->setIntensitySampling(energyBinWidth);
}

void StandardPipeline::setSpectralEffectsSampling(float energyBinWidth)
{
    _extSpectral->setSpectralSamplingResolution(energyBinWidth);
}

void StandardPipeline::setPoissonFixedSeed(uint seed)
{
    _extPoisson->setFixedSeed(seed);
}

void StandardPipeline::setPoissonRandomSeedMode()
{
    _extPoisson->setRandomSeedMode();
}

void StandardPipeline::setPoissonParallelizationMode(bool enabled)
{
    _extPoisson->setParallelizationEnabled(enabled);
}

void StandardPipeline::setRayCasterInterpolation(bool enabled)
{
    _projector->settings().interpolate = enabled;
}

void StandardPipeline::setRayCasterRaysPerPixel(const QSize &sampling)
{
    _projector->settings().raysPerPixel[0] = sampling.width();
    _projector->settings().raysPerPixel[1] = sampling.height();
}

void StandardPipeline::setRayCasterRaySampling(float sampling)
{
    _projector->settings().raySampling = sampling;
}

void StandardPipeline::setRayCasterVolumeUpSampling(uint upsamplingFactor)
{
    _projector->settings().volumeUpSampling = upsamplingFactor;
}

uint StandardPipeline::posAFS() const
{
    return 0;
}

uint StandardPipeline::posDetSat() const
{
    return uint(_arealFSEnabled)
            + uint(_spectralEffEnabled)
            + uint(_poissonEnabled);
}

uint StandardPipeline::posPoisson() const
{
    return _approxMode ? uint(_arealFSEnabled) + uint(_spectralEffEnabled)
                       : uint(_arealFSEnabled);
}

uint StandardPipeline::posSpectral() const
{
    return _approxMode ? uint(_arealFSEnabled)
                       : uint(_arealFSEnabled) + uint(_spectralEffEnabled);
}

void StandardPipeline::configure(const AcquisitionSetup& setup)
{
    _pipeline.configure(setup);
}

ProjectionData StandardPipeline::project(const VolumeData& volume)
{
    return _pipeline.project(volume);
}

ProjectionData StandardPipeline::projectComposite(const CompositeVolume& volume)
{
    return _pipeline.projectComposite(volume);
}

bool StandardPipeline::isLinear() const
{
    return _pipeline.isLinear();
}

void StandardPipeline::fromVariant(const QVariant& variant)
{
    AbstractProjector::fromVariant(variant);

    QVariantMap map = variant.toMap();

    enableArealFocalSpot(false);
    enableDetectorSaturation(false);
    enablePoissonNoise(false);
    enableSpectralEffects(false);

    _pipeline.setProjector(SerializationHelper::parseProjector(map.value("projector")));

    _extAFS->fromVariant(map.value("ext AFS"));
    _extDetSat->fromVariant(map.value("ext DetSat"));
    _extPoisson->fromVariant(map.value("ext Poisson"));
    _extSpectral->fromVariant(map.value("ext spectral"));

    enableArealFocalSpot(map.value("use areal focal spot").toBool());
    enableDetectorSaturation(map.value("use detector saturation").toBool());
    enablePoissonNoise(map.value("use poisson noise").toBool());
    enableSpectralEffects(map.value("use spectral effects").toBool());
}

QVariant StandardPipeline::toVariant() const
{
    QVariantMap ret = AbstractProjector::toVariant().toMap();

    ret.insert("#", "StandardPipeline");
    ret.insert("use areal focal spot", _arealFSEnabled);
    ret.insert("use detector saturation", _detSatEnabled);
    ret.insert("use poisson noise", _poissonEnabled);
    ret.insert("use spectral effects", _spectralEffEnabled);

    ret.insert("projector", _projector->toVariant());
    ret.insert("ext AFS", _extAFS->toVariant());
    ret.insert("ext DetSat", _extDetSat->toVariant());
    ret.insert("ext Poisson", _extPoisson->toVariant());
    ret.insert("ext spectral", _extSpectral->toVariant());

    return ret;
}

void StandardPipeline::SettingsPoissonNoise::setFixedSeed(uint seed)
{
    _ext->setFixedSeed(seed);
}

void StandardPipeline::SettingsPoissonNoise::setRandomSeedMode()
{
    _ext->setRandomSeedMode();
}

void StandardPipeline::SettingsPoissonNoise::setParallelizationMode(bool enabled)
{
    _ext->setParallelizationEnabled(enabled);
}

void StandardPipeline::SettingsAFS::setDiscretization(const QSize &discretization)
{
    _ext->setDiscretization(discretization);
}

void StandardPipeline::SettingsDetectorSaturation::setSamplingResolution(float energyBinWidth)
{
    _ext->setIntensitySampling(energyBinWidth);
}

void StandardPipeline::SettingsSpectralEffects::setSamplingResolution(float energyBinWidth)
{
    _ext->setSpectralSamplingResolution(energyBinWidth);
}

void StandardPipeline::SettingsRayCaster::setInterpolation(bool enabled)
{
    _proj->settings().interpolate = enabled;
}

void StandardPipeline::SettingsRayCaster::setRaysPerPixel(const QSize& sampling)
{
    _proj->settings().raysPerPixel[0] = sampling.width();
    _proj->settings().raysPerPixel[1] = sampling.height();
}

void StandardPipeline::SettingsRayCaster::setRaySampling(float sampling)
{
    _proj->settings().raySampling = sampling;
}

void StandardPipeline::SettingsRayCaster::setVolumeUpSampling(uint upsamplingFactor)
{
    _proj->settings().volumeUpSampling = upsamplingFactor;
}

} // namespace CTL
