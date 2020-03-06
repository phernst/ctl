#include "imageresampler.h"
#include "ocl/clfileloader.h"

#include <QDebug>

const std::string CL_FILE_NAME = "processing/imageResampler.cl"; //!< path to .cl file
const std::string CL_KERNEL_NAME = "resample"; //!< name of the OpenCL kernel function
const std::string CL_KERNEL_NAME_SUBSET_SAMPLER = "sample"; //!< name of the OpenCL kernel function
const std::string CL_PROGRAM_NAME = "imageResampler"; //!< OCL program name

namespace CTL {
namespace OCL {

ImageResampler::ImageResampler(const Chunk2D<float>& image,
                               const SamplingRange& rangeDim1,
                               const SamplingRange& rangeDim2,
                               uint oclDeviceNb)
    : _imgDim(image.dimensions())
    , _rangeDim1(rangeDim1)
    , _rangeDim2(rangeDim2)
    , _q(OpenCLConfig::instance().context(), OpenCLConfig::instance().devices()[oclDeviceNb])
    , _image2D(OpenCLConfig::instance().context(),
                  CL_MEM_READ_ONLY,
                  cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                  image.width(),
                  image.height())
    , _range1Buf(OpenCLConfig::instance().context(),
                 CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                 2 * sizeof(float))
    , _range2Buf(OpenCLConfig::instance().context(),
                 CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                 2 * sizeof(float))
{
    OCL::ClFileLoader clFile(CL_FILE_NAME);
    if(!clFile.isValid())
        throw std::runtime_error(CL_FILE_NAME + "\nis not readable");
    const auto clSourceCode = clFile.loadSourceCode();

    OpenCLConfig::instance().addKernel(CL_KERNEL_NAME, clSourceCode, CL_PROGRAM_NAME);
    OpenCLConfig::instance().addKernel(CL_KERNEL_NAME_SUBSET_SAMPLER, clSourceCode, CL_PROGRAM_NAME);

    // Create kernel
    try
    {
        _kernel = OpenCLConfig::instance().kernel(CL_KERNEL_NAME, CL_PROGRAM_NAME);
        _kernelSubsetSampler = OpenCLConfig::instance().kernel(CL_KERNEL_NAME_SUBSET_SAMPLER, CL_PROGRAM_NAME);

        cl::size_t<3> imgDim;
        imgDim[0] = image.dimensions().width;
        imgDim[1] = image.dimensions().height;
        imgDim[2] = 1;
        _q.enqueueWriteImage(_image2D, CL_TRUE, cl::size_t<3>(), imgDim, 0, 0,
                             const_cast<float*>(image.rawData()));
        _q.enqueueWriteBuffer(_range1Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim1);
        _q.enqueueWriteBuffer(_range2Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim2);

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    if(_kernel == nullptr || _kernelSubsetSampler == nullptr)
        throw std::runtime_error("kernel pointer not valid");
}

ImageResampler::ImageResampler(const Chunk2D<float>& image, uint oclDeviceNb)
    : ImageResampler(image,
                     { 0.0, float(image.width() - 1) },
                     { 0.0, float(image.height() - 1) },
                     oclDeviceNb)
{
}

void ImageResampler::setSamplingRanges(const SamplingRange& rangeDim1,
                                       const SamplingRange& rangeDim2)
{
    _rangeDim1 = rangeDim1;
    _rangeDim2 = rangeDim2;

    try
    {
        _q.enqueueWriteBuffer(_range1Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim1);
        _q.enqueueWriteBuffer(_range2Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim2);
    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }
}

Chunk2D<float> ImageResampler::image() const
{
    Chunk2D<float> ret(_imgDim);

    cl::size_t<3> imgDim;
    imgDim[0] = _imgDim.width;
    imgDim[1] = _imgDim.height;
    imgDim[2] = 1;

    ret.allocateMemory();
    _q.enqueueReadImage(_image2D, CL_TRUE, cl::size_t<3>(), imgDim, 0, 0, ret.rawData());

    return ret;
}

Chunk2D<float> ImageResampler::resample(const std::vector<float>& samplingPtsDim1,
                                        const std::vector<float>& samplingPtsDim2) const
{
    const auto nbSmpl1 = uint(samplingPtsDim1.size());
    const auto nbSmpl2 = uint(samplingPtsDim2.size());

    Chunk2D<float> ret(nbSmpl1, nbSmpl2);

    try
    {
        const auto& context = OpenCLConfig::instance().context();

        // write sampling points into buffers
        const auto memReadFlag = CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY;
        const auto size1 = nbSmpl1 * sizeof(float);
        const auto size2 = nbSmpl2 * sizeof(float);
        cl::Buffer smpl1Buf(context, memReadFlag, size1);
        cl::Buffer smpl2Buf(context, memReadFlag, size2);
        _q.enqueueWriteBuffer(smpl1Buf, CL_FALSE, 0, size1, samplingPtsDim1.data());
        _q.enqueueWriteBuffer(smpl2Buf, CL_FALSE, 0, size2, samplingPtsDim2.data());

        // buffer for resampled result
        auto imgSize = nbSmpl1 * nbSmpl2 * sizeof(float);
        cl::Buffer resampledImage(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, imgSize);

        // set kernel arguments
        _kernel->setArg(0, _range1Buf);
        _kernel->setArg(1, _range2Buf);
        _kernel->setArg(2, smpl1Buf);
        _kernel->setArg(3, smpl2Buf);
        _kernel->setArg(4, _image2D);
        _kernel->setArg(5, resampledImage);

        // run kernel
        _q.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(nbSmpl1, nbSmpl2));

        // read result
        ret.allocateMemory();
        _q.enqueueReadBuffer(resampledImage, CL_TRUE, 0, imgSize, ret.rawData());

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    return ret;
}

std::vector<float> ImageResampler::sample(const std::vector<Generic2DCoord>& samplingPts) const
{
    const auto nbSmpls = uint(samplingPts.size());
    std::vector<float> ret(nbSmpls);

    try
    {
        const auto& context = OpenCLConfig::instance().context();

        // write sampling points into buffers
        const auto memReadFlag = CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY;
        const auto size = 2 * nbSmpls * sizeof(float);
        cl::Buffer smplBuf(context, memReadFlag, size);
        _q.enqueueWriteBuffer(smplBuf, CL_FALSE, 0, size, samplingPts.data());

        // buffer for resampled result
        auto volSize = nbSmpls * sizeof(float);
        cl::Buffer resampledVolume(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, volSize);

        // set kernel arguments
        _kernelSubsetSampler->setArg(0, _range1Buf);
        _kernelSubsetSampler->setArg(1, _range2Buf);
        _kernelSubsetSampler->setArg(2, smplBuf);
        _kernelSubsetSampler->setArg(3, _image2D);
        _kernelSubsetSampler->setArg(4, resampledVolume);

        // run kernel
        _q.enqueueNDRangeKernel(*_kernelSubsetSampler, cl::NullRange, cl::NDRange(nbSmpls));

        // read result
        _q.enqueueReadBuffer(resampledVolume, CL_TRUE, 0, volSize, ret.data());

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    return ret;
}

/*!
 * Returns the dimensions (i.e. number of pixels) of the image managed by this instance.
 */
const Chunk2D<float>::Dimensions& ImageResampler::imgDim() const { return _imgDim; }

/*!
 * Returns the sampling range of the first dimension (boundary as first/last pixel).
 */
const SamplingRange& ImageResampler::rangeDim1() const { return _rangeDim1; }

/*!
 * Returns the sampling range of the second dimension (boundary as first/last pixel).
 */
const SamplingRange& ImageResampler::rangeDim2() const { return _rangeDim2; }

} // namespace OCL
} // namespace CTL
