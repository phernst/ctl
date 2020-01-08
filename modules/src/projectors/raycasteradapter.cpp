#include "raycasteradapter.h"
#include "components/genericdetector.h"
#include "acquisition/geometryencoder.h"

#include <iostream>

namespace CTL {
namespace OCL {

void RayCasterAdapter::configure(const AcquisitionSetup &setup,
                                 const AbstractProjectorConfig& config)
{
    // get projection matrices
    FullGeometry pMats = GeometryEncoder::encodeFullGeometry(setup);
    _pMatsVectorized.clear();
    for(const auto& viewPMats : pMats)
        for(const auto& modPMats : viewPMats)
            _pMatsVectorized.append(modPMats);

    // prepare RayCaster
    Q_ASSERT(dynamic_cast<const Config*>(&config));
    applyRayCasterConfig(dynamic_cast<const Config&>(config));

    auto detectorPixels = setup.system()->detector()->nbPixelPerModule();
    _rayCaster.setDetectorSize(detectorPixels.height(), detectorPixels.width());

    _viewDim.nbChannels = detectorPixels.width();
    _viewDim.nbRows = detectorPixels.height();
    _viewDim.nbModules = setup.system()->detector()->nbDetectorModules();
}

ProjectionData RayCasterAdapter::project(const VolumeData& volume)
{
    float volumeOff[3] = { 0.0, 0.0, 0.0 };

    uint nbVoxel[3] = { volume.nbVoxels().x, volume.nbVoxels().y, volume.nbVoxels().z };
    float voxelSize[3] = { volume.voxelSize().x, volume.voxelSize().y, volume.voxelSize().z };

    _rayCaster.setVolumeInfo(nbVoxel, voxelSize);
    _rayCaster.setVolumeOffset(volumeOff);

    auto result = _rayCaster.project({ _pMatsVectorized.begin(), _pMatsVectorized.end() },
                                     volume.constData());

    ProjectionData ret(_viewDim);
    ret.setDataFromVector(result);

    return ret;
}

void RayCasterAdapter::applyRayCasterConfig(const Config& rcConfig)
{
    std::cout << "configurate ray caster config" << std::endl;
    _rayCaster.setIncrement(rcConfig.increment_mm);
}

AbstractProjectorConfig* RayCasterAdapter::Config::clone() const
{
    return new Config(*this);
}

} // namespace OCL
} // namespace CTL
