#ifndef IMAGERESAMPLER_H
#define IMAGERESAMPLER_H

#include "img/chunk2d.h"
#include "ocl/openclconfig.h"
#include "processing/coordinates.h"

namespace CTL {
namespace OCL {

class ImageResampler
{
public:
    ImageResampler(const Chunk2D<float>& image, uint oclDeviceNb = 0);
    ImageResampler(const Chunk2D<float>& image,
                   const SamplingRange& rangeDim1,
                   const SamplingRange& rangeDim2,
                   uint oclDeviceNb = 0);

    void setSamplingRanges(const SamplingRange& rangeDim1,
                           const SamplingRange& rangeDim2);

    Chunk2D<float> image() const;
    Chunk2D<float> resample(const std::vector<float>& samplingPtsDim1,
                            const std::vector<float>& samplingPtsDim2) const;

    std::vector<float> sample(const std::vector<Generic2DCoord>& samplingPts) const;

    const Chunk2D<float>::Dimensions& imgDim() const;

private:
    Chunk2D<float>::Dimensions _imgDim; //!< Dimensions of the image

    SamplingRange _rangeDim1;
    SamplingRange _rangeDim2;

    cl::CommandQueue _q;
    cl::Kernel* _kernel;
    cl::Kernel* _kernelSubsetSampler;
    cl::Image2D _image2D;
    cl::Buffer _range1Buf;
    cl::Buffer _range2Buf;
};


} // namespace OCL
} // namespace CTL

#endif // IMAGERESAMPLER_H
