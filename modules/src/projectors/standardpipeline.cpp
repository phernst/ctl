#include "standardpipeline.h"

#include "raycasterprojector.h"
#include "arealfocalspotextension.h"
#include "detectorsaturationextension.h"
#include "poissonnoiseextension.h"
#include "spectraleffectsextension.h"

namespace CTL {

StandardPipeline::StandardPipeline()
{
    _pipeline.setProjector(makeProjector<OCL::RayCasterProjector>());

    _extAFS      = new ArealFocalSpotExtension;
    _extDetSat   = new DetectorSaturationExtension;
    _extPoisson  = new PoissonNoiseExtension;
    _extSpectral = new SpectralEffectsExtension;
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


}
