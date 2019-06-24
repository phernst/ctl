#ifndef VOLUMESLICER_H
#define VOLUMESLICER_H

#include "img/voxelvolume.h"
#include "mat/matrix.h"
#include "ocl/openclconfig.h"
#include "ocl/pinnedmem.h"

namespace CTL {
namespace OCL {

/*!
 * \class VolumeSlicer
 * \brief The VolumeSlicer class allows to slice VoxelVolume<float> data along arbitrary planes.
 *
 * This class allows to slice VoxelVolume<float> data along arbitrary planes. Computation is carried
 * out on an OpenCL device, making use of fast hardware interpolation on texture memory.
 *
 * To reslice volume data, create a VolumeSlicer instance by passing to its constructor a (constant)
 * reference to the volume that shall be resliced. The volume data will be transfered to the OpenCL
 * device immediately (stored internally as cl::Image3D).
 * Slices can then be obtained using slice(). For convenience, slices can be defined in two
 * different ways:
 * 1) by the plane's normal vector and distance to the origin, or
 * 2) by specifying the plane's angles w.r.t. the world coordinate system (in polar coordinates) and
 * its distance to the origin.
 *
 * By default, the dimensions (ie. number of pixels) and resolution (ie. pixel size) of resulting
 * slices are determined automatically - w.r.t. the volume's specs - using the following rules:
 *
 * Dimension (identical for x and y): ceil(sqrt(2) * max(nbVoxel.x, nbVoxel.y, nbVoxel.z)
 * Resolution (isotropic): min(voxelSize.x, voxelSize.y, voxelSize.z)
 *
 * If necessary, these specifications can be changed using setSliceDimensions() and
 * setSliceResolution(), respectively.
 */
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
    Chunk2D<float>::Dimensions _dim; //!< Dimensions of created slices
    float _reso; //!< Resolution (ie. pixel size) of created slices [in mm]
    cl::CommandQueue _q; //!< OpenCL command queue
    cl::Kernel* _kernel; //!< Pointer to OpenCL kernel
    cl::Image3D _volImage3D; //!< Volume data to be resliced
    cl::Buffer _homoBuf; //!< Buffer for homography transform
    cl::Buffer _sliceDimBuf; //!< Buffer for slice dimensions
    cl::Buffer _voxCornerBuf; //!< Buffer for volume corner
    PinnedBufHostRead<float> _sliceBuf; //!< Buffer for result (ie. slice), uses pinned memory
    VoxelVolume<float>::Dimensions _volDim; //!< Dimensions of the volume
    VoxelVolume<float>::Offset _volOffset; //!< Offset of the volume
    VoxelVolume<float>::VoxelSize _volVoxSize; //!< Voxel size of the volume

    mat::Matrix<3, 4> createInverseTransformationToXYPlane(const mat::Matrix<4, 1>& plane) const;
    static Chunk2D<float>::Dimensions sliceDim(const VoxelVolume<float>::Dimensions& volDim);
};

} // namespace OCL
} // namespace CTL

#endif // VOLUMESLICER_H
