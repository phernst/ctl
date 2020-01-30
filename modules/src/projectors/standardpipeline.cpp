#include "standardpipeline.h"

#include "raycasterprojector.h"
#include "arealfocalspotextension.h"
#include "detectorsaturationextension.h"
#include "poissonnoiseextension.h"
#include "spectraleffectsextension.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(StandardPipeline)

StandardPipeline::StandardPipeline()
{
    _extAFS      = new ArealFocalSpotExtension;
    _extDetSat   = new DetectorSaturationExtension;
    _extPoisson  = new PoissonNoiseExtension;
    _extSpectral = new SpectralEffectsExtension;

    _projector   = new OCL::RayCasterProjector;
    _pipeline.setProjector(_projector);
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

void StandardPipeline::setArealFocalSpotDiscretization(const QSize& discretization)
{
    _extAFS->setDiscretization(discretization);
}

void StandardPipeline::setSpectralEffectsSampling(float energyBinWidth)
{
    _extSpectral->setSpectralSamplingResolution(energyBinWidth);
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
    return uint(_arealFSEnabled)
            + uint(_spectralEffEnabled);
}

uint StandardPipeline::posSpectral() const
{
    return uint(_arealFSEnabled);
}

void StandardPipeline::configure(const AcquisitionSetup& setup)
{
    _pipeline.configure(setup);
}

ProjectionData StandardPipeline::project(const VolumeData& volume)
{
    return _pipeline.project(volume);
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

    delete _extAFS;
    delete _extDetSat;
    delete _extPoisson;
    delete _extSpectral;

    _pipeline.setProjector(SerializationHelper::parseProjector(map.value("projector")));

    _extAFS      = static_cast<ArealFocalSpotExtension*>(SerializationHelper::parseProjector(map.value("ext AFS")));
    _extDetSat   = static_cast<DetectorSaturationExtension*>(SerializationHelper::parseProjector(map.value("ext DetSat")));
    _extPoisson  = static_cast<PoissonNoiseExtension*>(SerializationHelper::parseProjector(map.value("ext Poisson")));
    _extSpectral = static_cast<SpectralEffectsExtension*>(SerializationHelper::parseProjector(map.value("ext spectral")));

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


}
