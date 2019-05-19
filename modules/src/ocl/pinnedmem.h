#ifndef PINNEDMEM_H
#define PINNEDMEM_H

#include "openclconfig.h"
#include <array>

namespace CTL {
namespace OCL {

namespace _pinned_mem_details {

// Pinned memory base class - manages command queue and host pointer to pinned memory
template<typename T>
class PinnedMem
{
public:
    PinnedMem(const cl::CommandQueue& queue);
    // access to pinned memory
    T* hostPtr() const;
    const cl::CommandQueue& queue() const;
    void setHostPtr(void* hostPtr);

protected:
    cl::CommandQueue _q; //!< device specific command queue
    T* _hostPtr; //!< pointer to pinned memory
};

// Base for Buffer classes
template<typename T>
class PinnedBufBase
{
public:
    PinnedBufBase(size_t nbElements, cl_mem_flags devAccess);

    const size_t& nbElements() const;
    cl::Buffer& devBuffer();

protected:
    cl::Buffer& pinnedBuffer();

private:
    size_t _nbElements;
    cl::Buffer _pinnedBuf;
    cl::Buffer _deviceBuf;
};

// Base for Image3D (float) classes
class PinnedImag3DBase
{
public:
    PinnedImag3DBase(size_t xDim, size_t yDim, size_t zDim, cl_mem_flags devAccess);

    const std::array<size_t, 3>& nbElements() const;
    cl::Image3D& devImage();

protected:
    cl::Image3D& pinnedImage();
    cl::size_t<3> zeros();
    cl::size_t<3> dimensions() const;

private:
    std::array<size_t, 3> _nbElements;
    cl::Image3D _pinnedImg;
    cl::Image3D _deviceImg;
};

} // namespace _pinned_mem_details

// Acess interfaces
// host write
template<typename T>
class AbstractHostWriteMem : virtual public _pinned_mem_details::PinnedMem<T>
{
    // copy from pinned memory to device (all elements)
    public:virtual void copyPinnedMemToDev(bool blocking, cl::Event* event) = 0;
    // copy from srcPtr to pinned memory (all elements)
    public:virtual void copyToPinnedMem(const T* srcPtr) = 0;

public:
    // first copy from srcPtr to pinned memory and then to device (all elements)
    void copyToDev(const T* srcPtr, bool blocking = true, cl::Event* event = nullptr);

    virtual ~AbstractHostWriteMem() = default;
};

// host read
template<typename T>
class AbstractHostReadMem : virtual public _pinned_mem_details::PinnedMem<T>
{
    // copy from pinned memory to device (all elements)
    public:virtual void copyDevToPinnedMem(bool blocking, cl::Event* event) = 0;
    // copy from srcPtr to pinned memory (all elements)
    public:virtual void copyFromPinnedMem(T* dstPtr) = 0;

public:
    // first copy from srcPtr to pinned memory and then to device (all elements)
    void copyFromDev(T* dstPtr, bool blocking = true, cl::Event* event = nullptr);

    virtual ~AbstractHostReadMem() = default;
};

// Buffer with host write access and device read access
template<typename T>
class PinnedBufHostWrite : public AbstractHostWriteMem<T>
                         , public _pinned_mem_details::PinnedBufBase<T>
{
public:
    PinnedBufHostWrite(size_t nbElements, const cl::CommandQueue& queue, bool deviceOnlyReads = true);

    // copy from pinned memory to device (all elements)
    void copyPinnedMemToDev(bool blocking = true, cl::Event* event = nullptr) override;
    // copy from srcPtr to pinned memory (all elements)
    void copyToPinnedMem(const T* srcPtr) override;
};

// Buffer with host read access and device write access
template<typename T>
class PinnedBufHostRead : public AbstractHostReadMem<T>
                        , public _pinned_mem_details::PinnedBufBase<T>
{
public:
    PinnedBufHostRead(size_t nbElements, const cl::CommandQueue& queue, bool deviceOnlyWrites = true);

    // copy from device to pinned memory (all elements)
    void copyDevToPinnedMem(bool blocking = true, cl::Event* event = nullptr) override;
    // copy from pinned memory to dstPtr (all elements)
    void copyFromPinnedMem(float* dstPtr) override;
};

// Image3D with host write access and device read access
class PinnedImg3DHostWrite : public AbstractHostWriteMem<float>
                           , public _pinned_mem_details::PinnedImag3DBase
{
    PinnedImg3DHostWrite(size_t xDim, size_t yDim, size_t zDim, const cl::CommandQueue& queue,
                         bool deviceOnlyReads = true);

    // copy from pinned memory to device (all elements)
    void copyPinnedMemToDev(bool blocking = true, cl::Event* event = nullptr) override;
    // copy from srcPtr to pinned memory (all elements)
    void copyToPinnedMem(const float* srcPtr) override;
};

// Image3D with host read access and device write access
class PinnedImg3DHostRead : public AbstractHostReadMem<float>
                          , public _pinned_mem_details::PinnedImag3DBase
{
    PinnedImg3DHostRead(size_t xDim, size_t yDim, size_t zDim, const cl::CommandQueue& queue,
                        bool deviceOnlyWrites = true);

    // copy from device to pinned memory (all elements)
    void copyDevToPinnedMem(bool blocking = true, cl::Event* event = nullptr) override;
    // copy from pinned memory to dstPtr (all elements)
    void copyFromPinnedMem(float *dstPtr) override;
};


// ### IMPLEMENTATION ###
// read-only Buffer
template<typename T>
PinnedBufHostWrite<T>::PinnedBufHostWrite(size_t nbElements, const cl::CommandQueue& queue, bool deviceOnlyReads)
    : _pinned_mem_details::PinnedMem<T>(queue)
    , _pinned_mem_details::PinnedBufBase<T>(nbElements, CL_MEM_HOST_WRITE_ONLY |
                                            (deviceOnlyReads ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE))
{
    setHostPtr(queue.enqueueMapBuffer(pinnedBuffer(), CL_TRUE, CL_MAP_WRITE, 0,
                                      sizeof(T) * this->nbElements()));
}

template<typename T>
void PinnedBufHostWrite<T>::copyPinnedMemToDev(bool blocking, cl::Event* event)
{
    queue().enqueueWriteBuffer(devBuffer(), blocking ? CL_TRUE : CL_FALSE, 0,
                               sizeof(T) * nbElements(), hostPtr(), nullptr, event);
}

template<typename T>
void PinnedBufHostWrite<T>::copyToPinnedMem(const T* srcPtr)
{
    std::copy_n(srcPtr, nbElements(), hostPtr());
}

// write-only Buffer
template<typename T>
PinnedBufHostRead<T>::PinnedBufHostRead(size_t nbElements, const cl::CommandQueue& queue, bool deviceOnlyWrites)
    : _pinned_mem_details::PinnedMem<T>(queue)
    , _pinned_mem_details::PinnedBufBase<T>(nbElements, CL_MEM_HOST_READ_ONLY |
                                            (deviceOnlyWrites ? CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE))
{
    setHostPtr(queue.enqueueMapBuffer(pinnedBuffer(), CL_TRUE, CL_MAP_READ, 0,
                                      sizeof(T) * this->nbElements()));
}

template<typename T>
void PinnedBufHostRead<T>::copyDevToPinnedMem(bool blocking, cl::Event *event)
{
    queue().enqueueReadBuffer(devBuffer(), blocking ? CL_TRUE : CL_FALSE, 0,
                              sizeof(T) * nbElements(), hostPtr(), nullptr, event);
}

template<typename T>
void PinnedBufHostRead<T>::copyFromPinnedMem(float* dstPtr)
{
    std::copy_n(hostPtr(), nbElements(), dstPtr);
}

// read-only Image3D
inline PinnedImg3DHostWrite::PinnedImg3DHostWrite(size_t xDim, size_t yDim, size_t zDim,
                                                  const cl::CommandQueue& queue, bool deviceOnlyReads)
    : _pinned_mem_details::PinnedMem<float>(queue)
    , _pinned_mem_details::PinnedImag3DBase(xDim, yDim, zDim, CL_MEM_HOST_WRITE_ONLY |
                                            (deviceOnlyReads ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE))
{
    size_t rowPitch, slicePitch;
    setHostPtr(queue.enqueueMapImage(pinnedImage(), CL_TRUE, CL_MAP_WRITE, zeros(), dimensions(),
                                     &rowPitch, &slicePitch));
}

inline void PinnedImg3DHostWrite::copyPinnedMemToDev(bool blocking, cl::Event* event)
{
    queue().enqueueWriteImage(devImage(), blocking ? CL_TRUE : CL_FALSE, zeros(), dimensions(), 0, 0,
                              hostPtr(), nullptr, event);
}

inline void PinnedImg3DHostWrite::copyToPinnedMem(const float* srcPtr)
{
    std::copy_n(srcPtr, nbElements()[0] * nbElements()[1] * nbElements()[2], hostPtr());
}

// write-only Image3D
inline PinnedImg3DHostRead::PinnedImg3DHostRead(size_t xDim, size_t yDim, size_t zDim,
                                                const cl::CommandQueue &queue,
                                                bool deviceOnlyWrites)
    : _pinned_mem_details::PinnedMem<float>(queue)
    , _pinned_mem_details::PinnedImag3DBase(xDim, yDim, zDim, CL_MEM_HOST_READ_ONLY |
                                            (deviceOnlyWrites ? CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE))
{
    size_t rowPitch, slicePitch;
    setHostPtr(queue.enqueueMapImage(pinnedImage(), CL_TRUE, CL_MAP_READ,
               zeros(), dimensions(), &rowPitch, &slicePitch));
}

inline void PinnedImg3DHostRead::copyDevToPinnedMem(bool blocking, cl::Event* event)
{
    queue().enqueueReadImage(devImage(), blocking ? CL_TRUE : CL_FALSE, zeros(), dimensions(), 0, 0,
                             hostPtr(), nullptr, event);
}

inline void PinnedImg3DHostRead::copyFromPinnedMem(float* dstPtr)
{
    std::copy_n(hostPtr(), nbElements()[0] * nbElements()[1] * nbElements()[2], dstPtr);
}

// Abstract read-only class
template<typename T>
void AbstractHostWriteMem<T>::copyToDev(const T* srcPtr, bool blocking, cl::Event* event)
{
    copyToPinnedMem(srcPtr);
    copyPinnedMemToDev(blocking, event);
}

// Abstract write-only class
template<typename T>
void AbstractHostReadMem<T>::copyFromDev(T *dstPtr, bool blocking, cl::Event* event)
{
    copyDevToPinnedMem(blocking, event);
    copyFromPinnedMem(dstPtr);
}


// ### DETAILS ###

namespace _pinned_mem_details {

// Generic pinned memory class
template<typename T>
PinnedMem<T>::PinnedMem(const cl::CommandQueue &queue)
    : _q(queue)
{}

template<typename T>
T* PinnedMem<T>::hostPtr() const { return _hostPtr; }

template<typename T>
void PinnedMem<T>::setHostPtr(void* hostPtr) { _hostPtr = reinterpret_cast<T*>(hostPtr); }

template<typename T>
const cl::CommandQueue& PinnedMem<T>::queue() const { return _q; }


// PinnedMemBufBase
template<typename T>
PinnedBufBase<T>::PinnedBufBase(size_t nbElements, cl_mem_flags devAccess)
    : _nbElements(nbElements)
    , _pinnedBuf(OpenCLConfig::instance().context(), CL_MEM_ALLOC_HOST_PTR | devAccess,
                 sizeof(T) * nbElements)
    , _deviceBuf(OpenCLConfig::instance().context(), devAccess, sizeof(T) * nbElements)
{
}

template<typename T>
const size_t& PinnedBufBase<T>::nbElements() const { return _nbElements; }

template<typename T>
cl::Buffer& PinnedBufBase<T>::devBuffer() { return _deviceBuf; }

template<typename T>
cl::Buffer& PinnedBufBase<T>::pinnedBuffer() { return _pinnedBuf; }


// PinnedMemImag3DBase
inline PinnedImag3DBase::PinnedImag3DBase(size_t xDim, size_t yDim, size_t zDim,
                                          cl_mem_flags devAccess)
    : _nbElements{ xDim, yDim, zDim }
    , _pinnedImg(OpenCLConfig::instance().context(), CL_MEM_ALLOC_HOST_PTR | devAccess,
                 cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                 _nbElements[0], _nbElements[1], _nbElements[2])
{
}

inline const std::array<size_t, 3>& PinnedImag3DBase::nbElements() const { return _nbElements; }

inline cl::Image3D& PinnedImag3DBase::devImage() { return _deviceImg; }

inline cl::Image3D& PinnedImag3DBase::pinnedImage() { return _pinnedImg; }

inline cl::size_t<3> PinnedImag3DBase::zeros()
{
    static cl::size_t<3> zeros;
    zeros[0] = 0;
    zeros[1] = 0;
    zeros[2] = 0;
    return zeros;
}

inline cl::size_t<3> PinnedImag3DBase::dimensions() const
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

#endif // PINNEDMEM_H
