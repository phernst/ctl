#ifndef VOLUMESLICER_H
#define VOLUMESLICER_H

#include "img/voxelvolume.h"
#include "mat/matrix.h"
#include "ocl/openclconfig.h"
#include "ocl/pinnedmem.h"

namespace CTL {
namespace OCL {

class VolumeSlicer
{
public:
    VolumeSlicer(const VoxelVolume<float>& volume, uint oclDeviceNb = 0);

    void setSliceDimensions(Chunk2D<float>::Dimensions dimensions);
    void setSliceResolution(float pixelResolution);

    Chunk2D<float>::Dimensions sliceDimensions() const;
    float sliceResolution() const;

    Chunk2D<float> slice(const mat::Matrix<3, 1>& planeUnitNormal,
                         double planeDistanceFromOrigin) const;
    Chunk2D<float> slice(double planeNormalAzimutAngle, double planeNormalPolarAngle,
                         double planeDistanceFromOrigin) const;

    const VoxelVolume<float>::Dimensions& volDim() const;
    const VoxelVolume<float>::Offset& volOffset() const;
    const VoxelVolume<float>::VoxelSize& volVoxSize() const;

private:
    Chunk2D<float>::Dimensions _dim;
    float _reso;
    cl::CommandQueue _q;
    cl::Kernel* _kernel;
    cl::Image3D _volImage3D;
    cl::Buffer _homoBuf;
    cl::Buffer _sliceDimBuf;
    cl::Buffer _voxCornerBuf;
    PinnedBufHostRead<float> _sliceBuf;
    VoxelVolume<float>::Dimensions _volDim;
    VoxelVolume<float>::Offset _volOffset;
    VoxelVolume<float>::VoxelSize _volVoxSize;

    mat::Matrix<3, 4> createInverseTransformationToXYPlane(const mat::Matrix<4, 1>& plane) const;
    static Chunk2D<float>::Dimensions sliceDim(const VoxelVolume<float>::Dimensions& volDim);
};

} // namespace OCL
} // namespace CTL

#endif // VOLUMESLICER_H
