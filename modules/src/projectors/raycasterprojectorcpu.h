#ifndef CTL_RAYCASTERPROJECTORCPU_H
#define CTL_RAYCASTERPROJECTORCPU_H

#include "abstractprojector.h"
#include "acquisition/viewgeometry.h"

namespace CTL {

class RayCasterProjectorCPU : public AbstractProjector
{
    class Settings
    {
    public:
        uint raysPerPixel[2] = { 1, 1 }; //!< number of ray per pixel in channel (x) and row (y) direction
        float raySampling = 0.3f; //!< fraction of voxel size that is used to increment position on ray
        bool interpolate = true; //!< enables interpolation of voxel value (attenuation) during ray casting
    };

public:
    // AbstractProjector interface
    void configure(const AcquisitionSetup &setup);
    ProjectionData project(const VolumeData &volume);

    Settings _settings; //!< settings of the projector
    SingleViewData::Dimensions _viewDim; //!< dimensions of a single view
    uint _volDim[3]; //!< cache for volume dimensions
    FullGeometry _pMats; //!< full set of projection matrices for all views and modules

private:
    SingleViewData computeView(const VolumeData& volume,
                               uint view,
                               const VoxelVolume<float>::Dimensions& volDim,
                               const VoxelVolume<float>::VoxelSize& voxelSize_mm,
                               const mat::Matrix<3,1>& volumeCorner,
                               QPair<uint,uint> raysPerPixel,
                               float increment_mm) const;
};

} // namespace CTL

#endif // CTL_RAYCASTERPROJECTORCPU_H
