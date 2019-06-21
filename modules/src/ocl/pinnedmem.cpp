#include "pinnedmem.h"

namespace CTL {
namespace OCL {

// read-only Image3D
PinnedImg3DHostWrite::PinnedImg3DHostWrite(size_t xDim, size_t yDim, size_t zDim,
                                           const cl::CommandQueue& queue,
                                           bool createDevBuffer, bool deviceOnlyReads)
    : _pinned_mem_details::PinnedImag3DBase(
          xDim,
          yDim,
          zDim,
          CL_MEM_HOST_WRITE_ONLY | (deviceOnlyReads ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE),
          CL_MAP_WRITE,
          createDevBuffer,
          queue)
{
}

void PinnedImg3DHostWrite::transferPinnedMemToDev(bool blocking, cl::Event* event) const
{
    queue().enqueueWriteImage(devImage(), blocking ? CL_TRUE : CL_FALSE, zeros(), dimensions(), 0,
                              0, hostPtr(), nullptr, event);
}

void PinnedImg3DHostWrite::writeToPinnedMem(const float* srcPtr) const
{
    std::copy_n(srcPtr, nbElements()[0] * nbElements()[1] * nbElements()[2], hostPtr());
}

// write-only Image3D
PinnedImg3DHostRead::PinnedImg3DHostRead(size_t xDim, size_t yDim, size_t zDim,
                                         const cl::CommandQueue& queue,
                                         bool createDevBuffer, bool deviceOnlyWrites)
    : _pinned_mem_details::PinnedImag3DBase(
          xDim,
          yDim,
          zDim,
          CL_MEM_HOST_READ_ONLY | (deviceOnlyWrites ? CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE),
          CL_MAP_READ,
          createDevBuffer,
          queue)
{
}

void PinnedImg3DHostRead::transferDevToPinnedMem(bool blocking, cl::Event* event) const
{
    queue().enqueueReadImage(devImage(), blocking ? CL_TRUE : CL_FALSE, zeros(), dimensions(), 0, 0,
                             hostPtr(), nullptr, event);
}

void PinnedImg3DHostRead::readFromPinnedMem(float* dstPtr) const
{
    std::copy_n(hostPtr(), nbElements()[0] * nbElements()[1] * nbElements()[2], dstPtr);
}

namespace _pinned_mem_details {

// PinnedMemImag3DBase
PinnedImag3DBase::PinnedImag3DBase(size_t xDim, size_t yDim, size_t zDim,
                                   cl_mem_flags devAccess, cl_map_flags hostAccess,
                                   bool createDevBuffer, const cl::CommandQueue& queue)
    : PinnedMem<float>(queue)
    , _nbElements{ xDim, yDim, zDim }
    , _pinnedImg(OpenCLConfig::instance().context(),
                 CL_MEM_ALLOC_HOST_PTR | devAccess,
                 cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                 _nbElements[0],
                 _nbElements[1],
                 _nbElements[2])
    , _deviceImg(createDevBuffer
                 ? cl::Image3D(OpenCLConfig::instance().context(),
                               devAccess,
                               cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                               _nbElements[0],
                               _nbElements[1],
                               _nbElements[2])
                 : cl::Image3D())
{
    size_t rowPitch, slicePitch;
    this->setHostPtr(reinterpret_cast<float*>(
                     queue.enqueueMapImage(_pinnedImg, CL_TRUE, hostAccess, zeros(), dimensions(),
                                           &rowPitch, &slicePitch)));
}

PinnedImag3DBase::~PinnedImag3DBase()
{
    if(this->hostPtr())
    {
        cl::Event e;
        this->queue().enqueueUnmapMemObject(_pinnedImg, this->hostPtr(), nullptr, &e);
        e.wait();
    }
}

const std::array<size_t, 3>& PinnedImag3DBase::nbElements() const { return _nbElements; }

const cl::Image3D& PinnedImag3DBase::devImage() const { return _deviceImg; }

cl::Image3D& PinnedImag3DBase::pinnedImage() { return _pinnedImg; }

const cl::size_t<3>& PinnedImag3DBase::zeros()
{
    static const cl::size_t<3> zeros; // auto initialized to zero
    return zeros;
}

cl::size_t<3> PinnedImag3DBase::dimensions() const
{
    cl::size_t<3> dims;
    dims[0] = _nbElements[0];
    dims[1] = _nbElements[1];
    dims[2] = _nbElements[2];
    return dims;
}

} // namespace _pinned_mem_details

} // namespace OCL
} // namespace CTL
