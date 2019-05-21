#include "pinnedmem.h"

namespace CTL {
namespace OCL {

// read-only Image3D
PinnedImg3DHostWrite::PinnedImg3DHostWrite(size_t xDim, size_t yDim, size_t zDim,
                                           const cl::CommandQueue& queue,
                                           bool createDevBuffer, bool deviceOnlyReads)
    : AbstractPinnedMemHostWrite(queue)
    , _pinned_mem_details::PinnedImag3DBase(
          xDim,
          yDim,
          zDim,
          CL_MEM_HOST_WRITE_ONLY | (deviceOnlyReads ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE),
          createDevBuffer)
{
    size_t rowPitch, slicePitch;
    setHostPtr(queue.enqueueMapImage(pinnedImage(), CL_TRUE, CL_MAP_WRITE, zeros(), dimensions(),
                                     &rowPitch, &slicePitch));
}

PinnedImg3DHostWrite::PinnedImg3DHostWrite(const PinnedImg3DHostWrite& usedPinnedMem,
                                           const cl::CommandQueue& queue,
                                           bool deviceOnlyReads)
    : AbstractPinnedMemHostWrite(queue)
    , _pinned_mem_details::PinnedImag3DBase(
          usedPinnedMem,
          CL_MEM_HOST_WRITE_ONLY | (deviceOnlyReads ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE))
{
    setHostPtr(usedPinnedMem.hostPtr());
}

void PinnedImg3DHostWrite::transferPinnedMemToDev(bool blocking, cl::Event* event)
{
    queue().enqueueWriteImage(devImage(), blocking ? CL_TRUE : CL_FALSE, zeros(), dimensions(), 0,
                              0, hostPtr(), nullptr, event);
}

void PinnedImg3DHostWrite::writeToPinnedMem(const float* srcPtr)
{
    std::copy_n(srcPtr, nbElements()[0] * nbElements()[1] * nbElements()[2], hostPtr());
}

// write-only Image3D
PinnedImg3DHostRead::PinnedImg3DHostRead(size_t xDim, size_t yDim, size_t zDim,
                                         const cl::CommandQueue& queue,
                                         bool createDevBuffer, bool deviceOnlyWrites)
    : AbstractPinnedMemHostRead<float>(queue)
    , _pinned_mem_details::PinnedImag3DBase(
          xDim,
          yDim,
          zDim,
          CL_MEM_HOST_READ_ONLY | (deviceOnlyWrites ? CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE),
          createDevBuffer)
{
    size_t rowPitch, slicePitch;
    setHostPtr(queue.enqueueMapImage(pinnedImage(), CL_TRUE, CL_MAP_READ, zeros(), dimensions(),
                                     &rowPitch, &slicePitch));
}

PinnedImg3DHostRead::PinnedImg3DHostRead(const PinnedImg3DHostRead& usedPinnedMem,
                                         const cl::CommandQueue& queue,
                                         bool deviceOnlyWrites)
    : AbstractPinnedMemHostRead<float>(queue)
    , _pinned_mem_details::PinnedImag3DBase(
          usedPinnedMem,
          CL_MEM_HOST_READ_ONLY | (deviceOnlyWrites ? CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE))
{
    setHostPtr(usedPinnedMem.hostPtr());
}

void PinnedImg3DHostRead::transferDevToPinnedMem(bool blocking, cl::Event* event)
{
    queue().enqueueReadImage(devImage(), blocking ? CL_TRUE : CL_FALSE, zeros(), dimensions(), 0, 0,
                             hostPtr(), nullptr, event);
}

void PinnedImg3DHostRead::readFromPinnedMem(float* dstPtr)
{
    std::copy_n(hostPtr(), nbElements()[0] * nbElements()[1] * nbElements()[2], dstPtr);
}

namespace _pinned_mem_details {

// PinnedMemImag3DBase
PinnedImag3DBase::PinnedImag3DBase(size_t xDim, size_t yDim, size_t zDim, cl_mem_flags devAccess, bool createDevBuffer)
    : _nbElements{ xDim, yDim, zDim }
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
}

PinnedImag3DBase::PinnedImag3DBase(const PinnedImag3DBase& usedPinnedMem, cl_mem_flags devAccess)
    : _nbElements{ usedPinnedMem.nbElements() }
 // , _pinnedImg -> remains a null object
    , _deviceImg(OpenCLConfig::instance().context(),
                 devAccess,
                 cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                 _nbElements[0],
                 _nbElements[1],
                 _nbElements[2])
{
}

const std::array<size_t, 3>& PinnedImag3DBase::nbElements() const { return _nbElements; }

cl::Image3D& PinnedImag3DBase::devImage() { return _deviceImg; }

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
