#include "volumeresampler.h"
#include "ocl/clfileloader.h"

#include <QDebug>

const std::string CL_FILE_NAME = "processing/volumeResampler.cl"; //!< path to .cl file
const std::string CL_KERNEL_NAME = "resample"; //!< name of the OpenCL kernel function
const std::string CL_KERNEL_NAME_SUBSET_SAMPLER = "sample"; //!< name of the OpenCL kernel function
const std::string CL_PROGRAM_NAME = "volumeResampler"; //!< OCL program name

namespace CTL {
namespace OCL {

VolumeResampler::VolumeResampler(const VoxelVolume<float>& volume,
                                 const SamplingRange& rangeDim1,
                                 const SamplingRange& rangeDim2,
                                 const SamplingRange& rangeDim3,
                                 uint oclDeviceNb)
    : _volDim(volume.dimensions())
    , _rangeDim1(rangeDim1)
    , _rangeDim2(rangeDim2)
    , _rangeDim3(rangeDim3)
    , _q(OpenCLConfig::instance().context(), OpenCLConfig::instance().devices()[oclDeviceNb])
    , _volImage3D(OpenCLConfig::instance().context(),
                  CL_MEM_READ_ONLY,
                  cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                  volume.dimensions().x,
                  volume.dimensions().y,
                  volume.dimensions().z)
    , _range1Buf(OpenCLConfig::instance().context(),
                 CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                 2 * sizeof(float))
    , _range2Buf(OpenCLConfig::instance().context(),
                 CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                 2 * sizeof(float))
    , _range3Buf(OpenCLConfig::instance().context(),
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

        cl::size_t<3> volDim;
        volDim[0] = volume.dimensions().x;
        volDim[1] = volume.dimensions().y;
        volDim[2] = volume.dimensions().z;
        _q.enqueueWriteImage(_volImage3D, CL_TRUE, cl::size_t<3>(), volDim, 0, 0,
                             const_cast<float*>(volume.rawData()));
        _q.enqueueWriteBuffer(_range1Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim1);
        _q.enqueueWriteBuffer(_range2Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim2);
        _q.enqueueWriteBuffer(_range3Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim3);

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    if(_kernel == nullptr || _kernelSubsetSampler == nullptr)
        throw std::runtime_error("kernel pointer not valid");
}

VolumeResampler::VolumeResampler(const VoxelVolume<float>& volume, uint oclDeviceNb)
    : VolumeResampler(
          volume,
          { volume.offset().x - 0.5f * volume.voxelSize().x * (volume.nbVoxels().x - 1),
            volume.offset().x + 0.5f * volume.voxelSize().x * (volume.nbVoxels().x - 1) },
          { volume.offset().y - 0.5f * volume.voxelSize().y * (volume.nbVoxels().y - 1),
            volume.offset().y + 0.5f * volume.voxelSize().y * (volume.nbVoxels().y - 1) },
          { volume.offset().z - 0.5f * volume.voxelSize().z * (volume.nbVoxels().z - 1),
            volume.offset().z + 0.5f * volume.voxelSize().z * (volume.nbVoxels().z - 1) },
          oclDeviceNb)
{
    qDebug().noquote() << "ranges in VolumeResampler:\n"
                       << "azi:" << _rangeDim1.start() << _rangeDim1.end() << "\n"
                       << "pol:" << _rangeDim2.start() << _rangeDim2.end() << "\n"
                       << "dst:" << _rangeDim3.start() << _rangeDim3.end();
}

void VolumeResampler::setSamplingRanges(const SamplingRange& rangeDim1,
                                        const SamplingRange& rangeDim2,
                                        const SamplingRange& rangeDim3)
{
    _rangeDim1 = rangeDim1;
    _rangeDim2 = rangeDim2;
    _rangeDim3 = rangeDim3;

    try
    {
        _q.enqueueWriteBuffer(_range1Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim1);
        _q.enqueueWriteBuffer(_range2Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim2);
        _q.enqueueWriteBuffer(_range3Buf, CL_FALSE, 0, 2 * sizeof(float), &_rangeDim3);
    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }
}

const SamplingRange& VolumeResampler::rangeDim1() const { return _rangeDim1; }

const SamplingRange& VolumeResampler::rangeDim2() const { return _rangeDim2; }

const SamplingRange& VolumeResampler::rangeDim3() const { return _rangeDim3; }

VoxelVolume<float> VolumeResampler::resample(const std::vector<float>& samplingPtsDim1,
                                             const std::vector<float>& samplingPtsDim2,
                                             const std::vector<float>& samplingPtsDim3) const
{
    const auto nbSmpl1 = uint(samplingPtsDim1.size());
    const auto nbSmpl2 = uint(samplingPtsDim2.size());
    const auto nbSmpl3 = uint(samplingPtsDim3.size());

    VoxelVolume<float> ret(nbSmpl1, nbSmpl2, nbSmpl3);

    try
    {
        const auto& context = OpenCLConfig::instance().context();

        // write sampling points into buffers
        const auto memReadFlag = CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY;
        const auto size1 = nbSmpl1 * sizeof(float);
        const auto size2 = nbSmpl2 * sizeof(float);
        const auto size3 = nbSmpl3 * sizeof(float);
        cl::Buffer smpl1Buf(context, memReadFlag, size1);
        cl::Buffer smpl2Buf(context, memReadFlag, size2);
        cl::Buffer smpl3Buf(context, memReadFlag, size3);
        _q.enqueueWriteBuffer(smpl1Buf, CL_FALSE, 0, size1, samplingPtsDim1.data());
        _q.enqueueWriteBuffer(smpl2Buf, CL_FALSE, 0, size2, samplingPtsDim2.data());
        _q.enqueueWriteBuffer(smpl3Buf, CL_FALSE, 0, size3, samplingPtsDim3.data());

        // buffer for resampled result
        auto volSize = nbSmpl1 * nbSmpl2 * nbSmpl3 * sizeof(float);
        cl::Buffer resampledVolume(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, volSize);

        // set kernel arguments
        _kernel->setArg(0, _range1Buf);
        _kernel->setArg(1, _range2Buf);
        _kernel->setArg(2, _range3Buf);
        _kernel->setArg(3, smpl1Buf);
        _kernel->setArg(4, smpl2Buf);
        _kernel->setArg(5, smpl3Buf);
        _kernel->setArg(6, _volImage3D);
        _kernel->setArg(7, resampledVolume);

        // run kernel
        _q.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(nbSmpl1, nbSmpl2, nbSmpl3));

        // read result
        ret.allocateMemory();
        _q.enqueueReadBuffer(resampledVolume, CL_TRUE, 0, volSize, ret.rawData());

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    return ret;
}

std::vector<float> VolumeResampler::sample(const std::vector<Generic3DCoord> &samplingPts) const
{
    const auto nbSmpls = uint(samplingPts.size());
    std::vector<float> ret(nbSmpls);

    try
    {
        const auto& context = OpenCLConfig::instance().context();

        // write sampling points into buffers
        const auto memReadFlag = CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY;
        const auto size = 3 * nbSmpls * sizeof(float);
        cl::Buffer smplBuf(context, memReadFlag, size);
        _q.enqueueWriteBuffer(smplBuf, CL_FALSE, 0, size, samplingPts.data());

        // buffer for resampled result
        auto volSize = nbSmpls * sizeof(float);
        cl::Buffer resampledVolume(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, volSize);

        // set kernel arguments
        _kernelSubsetSampler->setArg(0, _range1Buf);
        _kernelSubsetSampler->setArg(1, _range2Buf);
        _kernelSubsetSampler->setArg(2, _range3Buf);
        _kernelSubsetSampler->setArg(3, smplBuf);
        _kernelSubsetSampler->setArg(4, _volImage3D);
        _kernelSubsetSampler->setArg(5, resampledVolume);

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

std::vector<float> VolumeResampler::sample(const cl::Buffer& coord3dBuffer) const
{
    std::vector<float> ret;

    try
    {
        cl_int error;
        const auto bytesOfBuffer = coord3dBuffer.getInfo<CL_MEM_SIZE>(&error);
        constexpr auto bytesOfCoordTriple = sizeof(float) * 3;
        if(error != CL_SUCCESS || bytesOfBuffer % bytesOfCoordTriple)
            throw std::runtime_error("VolumeResampler::sample: invalid CL buffer for coordinates.");

        // buffer for resampled result
        const auto nbSmpls = bytesOfBuffer / bytesOfCoordTriple;
        const auto volSize = nbSmpls * sizeof(float);
        cl::Buffer resampledVolume(OpenCLConfig::instance().context(),
                                   CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, volSize);

        // set kernel arguments
        _kernelSubsetSampler->setArg(0, _range1Buf);
        _kernelSubsetSampler->setArg(1, _range2Buf);
        _kernelSubsetSampler->setArg(2, _range3Buf);
        _kernelSubsetSampler->setArg(3, coord3dBuffer);
        _kernelSubsetSampler->setArg(4, _volImage3D);
        _kernelSubsetSampler->setArg(5, resampledVolume);

        // run kernel
        _q.enqueueNDRangeKernel(*_kernelSubsetSampler, cl::NullRange, cl::NDRange(nbSmpls));

        // read result
        ret.resize(nbSmpls);
        _q.enqueueReadBuffer(resampledVolume, CL_TRUE, 0, volSize, ret.data());

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    return ret;
}

VoxelVolume<float> VolumeResampler::volume() const
{
    VoxelVolume<float> ret(_volDim, volVoxSize());
    ret.setVolumeOffset(volOffset());

    cl::size_t<3> volDim;
    volDim[0] = _volDim.x;
    volDim[1] = _volDim.y;
    volDim[2] = _volDim.z;

    ret.allocateMemory();
    _q.enqueueReadImage(_volImage3D, CL_TRUE, cl::size_t<3>(), volDim, 0, 0, ret.rawData());

    return ret;
}

/*!
 * Returns the dimensions (i.e. number of voxels) of the volume managed by this instance.
 */
const VoxelVolume<float>::Dimensions& VolumeResampler::volDim() const
{
    return _volDim;
}

/*!
 * Returns the offset (in mm) of the volume managed by this instance.
 */
VoxelVolume<float>::Offset VolumeResampler::volOffset() const
{
    return { _rangeDim1.center(), _rangeDim2.center(), _rangeDim3.center() };
}

/*!
 * Returns the size of the voxels in the volume managed by this instance.
 */
VoxelVolume<float>::VoxelSize VolumeResampler::volVoxSize() const
{
    return { _rangeDim1.spacing(_volDim.x),
             _rangeDim2.spacing(_volDim.y),
             _rangeDim3.spacing(_volDim.z) };
}

} // namespace OCL
} // namespace CTL
