#ifndef RAYCASTER_H
#define RAYCASTER_H

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#if defined(__APPLE__) || defined(__MACOSX)
#include "OpenCL/cl.hpp"
#else
#include <CL/cl.hpp>
#endif

#include "mat/matrix_types.h"

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
    void setVolumeInfo(const uint (&nbVoxel)[3], const float (&voxSize)[3]);

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

} // namespace OCL
} // namespace CTL

#endif // RAYCASTER_H
