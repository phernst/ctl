#ifndef RADONTRANSFORM3D_H
#define RADONTRANSFORM3D_H

#include "img/voxelvolume.h"
#include "mat/matrix.h"
#include "ocl/openclconfig.h"
#include "ocl/pinnedmem.h"

namespace CTL {
namespace OCL {

/*!
 * \class RadonTransform3D
 * \brief Allows to compute the 3D Radon transform of VoxelVolume<float> data.
 *
 * This class allows to compute the 3D Radon transform of VoxelVolume<float> data. The 3D Radon
 * transform describes a volume by all its plane integrals.
 * Computation is carried out on an OpenCL device, making use of fast hardware interpolation on
 * texture memory.
 *
 * To compute the 3D Radon transform of volume data, create a RadonTransform3D instance by passing
 * to its constructor a (constant) reference to the volume that you want to compute the transform
 * of. The volume data will be transfered to the OpenCL device immediately (stored internally as
 * cl::Image3D).
 *
 * The Radon transform of the volume can be computed for a given set of sampling points using
 * sampleTransform(). This will return the plane integral for all combinations of angles and
 * distances passed to sampleTransform(). This method supports multi-GPU and will automatically
 * split the task (w.r.t. to the distance samples) across all available devices (as returned by
 * OpenCLConfig::instance().devices()).
 *
 * Individual plane integrals (i.e. individual samples in the 3D Radon space) can be computed using
 * planeIntegral(). For convenience, the integration plane can be defined in two different ways:
 * 1) by the plane's normal vector and distance to the origin, or
 * 2) by specifying the plane's angles w.r.t. the world coordinate system (in polar coordinates) and
 * its distance to the origin.
 *
 * By default, the dimensions (ie. number of pixels) and resolution (ie. pixel size) within the
 * integration plane are determined automatically - w.r.t. the volume's specs - using the following
 * rules:
 *
 * Dimension (isotropic): nextMultipleOf16(ceil(sqrt(2) * max(nbVoxel.x, nbVoxel.y, nbVoxel.z))
 * Resolution (isotropic): min(voxelSize.x, voxelSize.y, voxelSize.z)
 *
 * The nextMultipleOf16 operation is required for computation reasons, where the plane integral is
 * carried out internally by computing sums of 16x16 pixel patches (due to limitations on local
 * memory).
 *
 * If necessary, the computation resolution can be changed using setSliceResolution(). Note that
 * this only changes the resolution with which plane integrals are computed. Reducing resolution
 * can speed up computation, for it results in less pixels required to sample the plane. However,
 * this can lead to loss in accuracy.
 */
class RadonTransform3D
{
public:
    RadonTransform3D(const VoxelVolume<float>& volume);

    // setter methods
    void setSliceResolution(float pixelResolution);

    // getter methods
    Chunk2D<float>::Dimensions sliceDimensions() const;
    float sliceResolution() const;
    const VoxelVolume<float>::Dimensions& volDim() const;
    const VoxelVolume<float>::Offset& volOffset() const;
    const VoxelVolume<float>::VoxelSize& volVoxSize() const;

    // sampling of several plane integrals
    VoxelVolume<float> sampleTransform(const std::vector<double>& azimuthAngleSampling,
                                       const std::vector<double>& polarAngleSampling,
                                       const std::vector<double>& distanceSampling) const;
    // single plane integral
    float planeIntegral(const mat::Matrix<3, 1>& planeUnitNormal,
                        double planeDistanceFromOrigin) const;
    float planeIntegral(double planeNormalAzimutAngle, double planeNormalPolarAngle,
                        double planeDistanceFromOrigin) const;

private:
    // types
    typedef std::pair<const float*, std::unique_ptr<cl::Event>> DeviceResult;

    struct Parameters
    {
        Chunk2D<float>::Dimensions dim;
        float reso;
        VoxelVolume<float>::Dimensions volDim;
        VoxelVolume<float>::Offset volOffset;
        VoxelVolume<float>::VoxelSize volVoxSize;

        uint nbPatches() const;
    };

    class SingleGPU
    {
    public:
        SingleGPU(const VoxelVolume<float>& volume, const Parameters& params, uint oclDeviceNb);

        void sliceDimensionsChanged();

        DeviceResult planeIntegral(const mat::Matrix<3, 1>& planeUnitNormal,
                                   double planeDistanceFromOrigin) const;
        DeviceResult planeIntegral(double planeNormalAzimutAngle, double planeNormalPolarAngle,
                                   double planeDistanceFromOrigin) const;

        const float* resultArray() const;

    private:
        const Parameters& _p;
        cl::CommandQueue _q;
        cl::Kernel* _kernel;
        cl::Image3D _volImage3D;
        cl::Buffer _homoBuf;
        cl::Buffer _sliceDimBuf;
        cl::Buffer _voxCornerBuf;
        PinnedBufHostRead<float> _resultBuf;

        mat::Matrix<3, 4> transformXYPlaneToPlane(const mat::Matrix<4, 1>& plane) const;
    };

    // member variables
    Parameters _p;
    std::vector<SingleGPU> _tasks;

    // static functions
    static uint nextMultipleOfN(uint value, uint N);
    static Chunk2D<float>::Dimensions sliceDim(const VoxelVolume<float>::Dimensions& volDim);
};

} // namespace OCL
} // namespace CTL

#endif // RADONTRANSFORM3D_H
