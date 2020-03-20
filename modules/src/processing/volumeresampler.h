#ifndef CTL_VOLUMERESAMPLER_H
#define CTL_VOLUMERESAMPLER_H

#include "img/voxelvolume.h"
#include "ocl/openclconfig.h"
#include "processing/coordinates.h"

namespace CTL {
namespace OCL {

class VolumeResampler
{
public:
    explicit VolumeResampler(const VoxelVolume<float>& volume, uint oclDeviceNb = 0);
    VolumeResampler(const VoxelVolume<float>& volume,
                    const SamplingRange& rangeDim1,
                    const SamplingRange& rangeDim2,
                    const SamplingRange& rangeDim3,
                    uint oclDeviceNb = 0);

    const SamplingRange& rangeDim1() const;
    const SamplingRange& rangeDim2() const;
    const SamplingRange& rangeDim3() const;

    VoxelVolume<float> resample(const std::vector<float>& samplingPtsDim1,
                                const std::vector<float>& samplingPtsDim2,
                                const std::vector<float>& samplingPtsDim3) const;

    std::vector<float> sample(const std::vector<Generic3DCoord>& samplingPts) const;
    std::vector<float> sample(const cl::Buffer& coord3dBuffer) const;

    void setSamplingRanges(const SamplingRange& rangeDim1,
                           const SamplingRange& rangeDim2,
                           const SamplingRange& rangeDim3);

    VoxelVolume<float> volume() const;

    const VoxelVolume<float>::Dimensions& volDim() const;
    VoxelVolume<float>::Offset volOffset() const;
    VoxelVolume<float>::VoxelSize volVoxSize() const;

private:
    VoxelVolume<float>::Dimensions _volDim; //!< Dimensions of the volume

    SamplingRange _rangeDim1;
    SamplingRange _rangeDim2;
    SamplingRange _rangeDim3;

    cl::CommandQueue _q;
    cl::Kernel* _kernel;
    cl::Kernel* _kernelSubsetSampler;
    cl::Image3D _volImage3D;
    cl::Buffer _range1Buf;
    cl::Buffer _range2Buf;
    cl::Buffer _range3Buf;
};


} // namespace OCL
} // namespace CTL

#endif // CTL_VOLUMERESAMPLER_H
