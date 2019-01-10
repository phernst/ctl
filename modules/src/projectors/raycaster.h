#ifndef RAYCASTER_H
#define RAYCASTER_H

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include "mat/matrix_types.h"
#include <CL/cl.hpp>

namespace CTL {
namespace OCL {

class RayCaster
{
public:
    RayCaster();
    std::vector<float> project(const std::vector<ProjectionMatrix>& Pmats,
                               const std::vector<float>& volume);

    void setDetectorSize(uint nbRows, uint nbColumns);
    void setIncrement(float incrementMM);
    void setVolumeOffset(const float (&offset)[3]);
    void setVolumeInfo(const uint (&nbVoxel)[3], const float (&voxelSize)[3]);

private:
    cl::Context context;
    cl::Program program;
    std::vector<cl::Device> device;

    cl_uint detectorColumns;
    cl_uint detectorRows;
    cl_float increment_mm;
    cl::size_t<3> volDim;
    cl_float3 volOffset;
    cl_float3 voxelSize;

    void initOpenCL();

    cl_float3 volumeCorner() const;
};

inline void RayCaster::setDetectorSize(uint nbRows, uint nbColumns)
{
    detectorRows = cl_uint(nbRows);
    detectorColumns = cl_uint(nbColumns);
}

inline void RayCaster::setIncrement(float incrementMM) { increment_mm = cl_float(incrementMM); }

inline void RayCaster::setVolumeOffset(const float (&offset)[3])
{
    volOffset = { { offset[0], offset[1], offset[2] } };
}

inline void RayCaster::setVolumeInfo(const uint (&nbVoxel)[3], const float (&vSize)[3])
{
    volDim[0] = nbVoxel[0];
    volDim[1] = nbVoxel[1];
    volDim[2] = nbVoxel[2];
    voxelSize = { { vSize[0], vSize[1], vSize[2] } };
}

} // namespace OCL
} // namespace CTL

#endif // RAYCASTER_H
