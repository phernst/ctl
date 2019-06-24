#ifndef RADONTRANSFORM3D_H
#define RADONTRANSFORM3D_H

#include "img/voxelvolume.h"
#include "mat/matrix.h"
#include "ocl/openclconfig.h"
#include "ocl/pinnedmem.h"

namespace CTL {
namespace OCL {

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
