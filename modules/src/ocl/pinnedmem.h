#ifndef PINNEDMEM_H
#define PINNEDMEM_H

#include "openclconfig.h"
#include <algorithm>
#include <array>
#include <future>

namespace CTL {
namespace OCL {

// Helper classes
// ==============

namespace _pinned_mem_details {

// Pinned memory base class - manages host pointer and command queue to pinned memory
template <typename T>
class PinnedMem
{
public:
    PinnedMem(const cl::CommandQueue& queue);

    PinnedMem(PinnedMem&& other);
    PinnedMem& operator=(PinnedMem&& other);
    // non-copyable
    PinnedMem(const PinnedMem& other) = delete;
    PinnedMem& operator=(const PinnedMem& other) = delete;

    T* hostPtr() const;
    const cl::CommandQueue& queue() const;

protected:
    void setHostPtr(T* hostPtr);

private:
    T* _hostPtr; //!< pointer to pinned memory
    cl::CommandQueue _q; //!< device specific command queue
};

// Base for Buffer classes
template <typename T>
class PinnedBufBase : public PinnedMem<T>
{
public:
    PinnedBufBase(size_t nbElements,
                  cl_mem_flags devAccess, cl_map_flags hostAccess,
                  bool createDevBuffer, const cl::CommandQueue& queue);

    PinnedBufBase(PinnedBufBase&&) = default;
    PinnedBufBase& operator=(PinnedBufBase&&) = default;
    ~PinnedBufBase();

    const size_t& nbElements() const;
    const cl::Buffer& devBuffer() const;

protected:
    cl::Buffer& pinnedBuffer();

private:
    size_t _nbElements;
    cl::Buffer _pinnedBuf;
    cl::Buffer _deviceBuf;
};

// Base for Image3D (with float/intensity format) classes
class PinnedImag3DBase : public PinnedMem<float>
{
public:
    PinnedImag3DBase(size_t xDim, size_t yDim, size_t zDim,
                     cl_mem_flags devAccess, cl_map_flags hostAccess,
                     bool createDevBuffer, const cl::CommandQueue& queue);

    PinnedImag3DBase(PinnedImag3DBase&&) = default;
    PinnedImag3DBase& operator=(PinnedImag3DBase&&) = default;
    ~PinnedImag3DBase();

    const std::array<size_t, 3>& nbElements() const;
    const cl::Image3D& devImage() const;

protected:
    cl::Image3D& pinnedImage();
    cl::size_t<3> dimensions() const;
    static const cl::size_t<3>& zeros();

private:
    std::array<size_t, 3> _nbElements;
    cl::Image3D _pinnedImg;
    cl::Image3D _deviceImg;
};

} // namespace _pinned_mem_details

// Access interfaces
// =================

// host write
template <typename T>
class AbstractPinnedMemHostWrite
{
    // copy from pinned memory to device (all elements)
    public:virtual void transferPinnedMemToDev(bool blocking = true, cl::Event* event = nullptr) const = 0;
    // copy from srcPtr to pinned memory (all elements)
    public:virtual void writeToPinnedMem(const T* srcPtr) const = 0;

public:
    AbstractPinnedMemHostWrite() = default;
    AbstractPinnedMemHostWrite(AbstractPinnedMemHostWrite&&) = default;
    AbstractPinnedMemHostWrite& operator=(AbstractPinnedMemHostWrite&&) = default;
    virtual ~AbstractPinnedMemHostWrite() = default;

    // first copy from srcPtr to pinned memory and then to device (all elements)
    void writeToDev(const T* srcPtr, bool blocking = true, cl::Event* event = nullptr) const;
};

// host read
template <typename T>
class AbstractPinnedMemHostRead
{
    // copy from pinned memory to device (all elements)
    public:virtual void transferDevToPinnedMem(bool blocking = true, cl::Event* event = nullptr) const = 0;
    // copy from srcPtr to pinned memory (all elements)
    public:virtual void readFromPinnedMem(T* dstPtr) const = 0;

public:
    AbstractPinnedMemHostRead() = default;
    AbstractPinnedMemHostRead(AbstractPinnedMemHostRead&&) = default;
    AbstractPinnedMemHostRead& operator=(AbstractPinnedMemHostRead&&) = default;
    virtual ~AbstractPinnedMemHostRead() = default;

    // first copy from srcPtr to pinned memory and then to device (all elements)
    void readFromDev(T* dstPtr) const;
    std::future<T*> readFromDevAsync(T* dstPtr) const;
};

// Concrete subclasses providing pinned memory for Buffer/Image3D with host read/write access
// ==========================================================================================

// Buffer with host write access and device read access
template <typename T>
class PinnedBufHostWrite : public AbstractPinnedMemHostWrite<T>,
                           public _pinned_mem_details::PinnedBufBase<T>
{
public:
    PinnedBufHostWrite(size_t nbElements,
                       const cl::CommandQueue& queue,
                       bool createDevBuffer = true,
                       bool deviceOnlyReads = true);

    // copy from pinned memory to device (all elements)
    void transferPinnedMemToDev(bool blocking = true, cl::Event* event = nullptr) const override;
    // copy from srcPtr to pinned memory (all elements)
    void writeToPinnedMem(const T* srcPtr) const override;
};

// Buffer with host read access and device write access
template <typename T>
class PinnedBufHostRead : public AbstractPinnedMemHostRead<T>,
                          public _pinned_mem_details::PinnedBufBase<T>
{
public:
    PinnedBufHostRead(size_t nbElements,
                      const cl::CommandQueue& queue,
                      bool createDevBuffer = true,
                      bool deviceOnlyWrites = true);

    // copy from device to pinned memory (all elements)
    void transferDevToPinnedMem(bool blocking = true, cl::Event* event = nullptr) const override;
    // copy from pinned memory to dstPtr (all elements)
    void readFromPinnedMem(T* dstPtr) const override;
};

// Image3D with host write access and device read access
class PinnedImg3DHostWrite : public AbstractPinnedMemHostWrite<float>,
                             public _pinned_mem_details::PinnedImag3DBase
{
public:
    PinnedImg3DHostWrite(size_t xDim,
                         size_t yDim,
                         size_t zDim,
                         const cl::CommandQueue& queue,
                         bool createDevBuffer = true,
                         bool deviceOnlyReads = true);

    // copy from pinned memory to device (all elements)
    void transferPinnedMemToDev(bool blocking = true, cl::Event* event = nullptr) const override;
    // copy from srcPtr to pinned memory (all elements)
    void writeToPinnedMem(const float* srcPtr) const override;
};

// Image3D with host read access and device write access
class PinnedImg3DHostRead : public AbstractPinnedMemHostRead<float>,
                            public _pinned_mem_details::PinnedImag3DBase
{
public:
    PinnedImg3DHostRead(size_t xDim,
                        size_t yDim,
                        size_t zDim,
                        const cl::CommandQueue& queue,
                        bool createDevBuffer = true,
                        bool deviceOnlyWrites = true);

    // copy from device to pinned memory (all elements)
    void transferDevToPinnedMem(bool blocking = true, cl::Event* event = nullptr) const override;
    // copy from pinned memory to dstPtr (all elements)
    void readFromPinnedMem(float* dstPtr) const override;
};

// Template implementation
// =======================

// read-only Buffer
template <typename T>
PinnedBufHostWrite<T>::PinnedBufHostWrite(size_t nbElements,
                                          const cl::CommandQueue& queue,
                                          bool createDevBuffer,
                                          bool deviceOnlyReads)
    : _pinned_mem_details::PinnedBufBase<T>(
          nbElements,
          CL_MEM_HOST_WRITE_ONLY | (deviceOnlyReads ? CL_MEM_READ_ONLY : CL_MEM_READ_WRITE),
          CL_MAP_WRITE,
          createDevBuffer,
          queue)
{
}

template <typename T>
void PinnedBufHostWrite<T>::transferPinnedMemToDev(bool blocking, cl::Event* event) const
{
    this->queue().enqueueWriteBuffer(this->devBuffer(), blocking ? CL_TRUE : CL_FALSE, 0,
                                     sizeof(T) * this->nbElements(), this->hostPtr(), nullptr,
                                     event);
}

template <typename T>
void PinnedBufHostWrite<T>::writeToPinnedMem(const T* srcPtr) const
{
    std::copy_n(srcPtr, this->nbElements(), this->hostPtr());
}

// write-only Buffer
template <typename T>
PinnedBufHostRead<T>::PinnedBufHostRead(size_t nbElements,
                                        const cl::CommandQueue& queue,
                                        bool createDevBuffer,
                                        bool deviceOnlyWrites)
    : _pinned_mem_details::PinnedBufBase<T>(
          nbElements,
          CL_MEM_HOST_READ_ONLY | (deviceOnlyWrites ? CL_MEM_WRITE_ONLY : CL_MEM_READ_WRITE),
          CL_MAP_READ,
          createDevBuffer,
          queue)
{
}

template <typename T>
void PinnedBufHostRead<T>::transferDevToPinnedMem(bool blocking, cl::Event* event) const
{
    this->queue().enqueueReadBuffer(this->devBuffer(), blocking ? CL_TRUE : CL_FALSE, 0,
                                    sizeof(T) * this->nbElements(), this->hostPtr(), nullptr,
                                    event);
}

template <typename T>
void PinnedBufHostRead<T>::readFromPinnedMem(T* dstPtr) const
{
    std::copy_n(this->hostPtr(), this->nbElements(), dstPtr);
}


// Abstract read-only class

template <typename T>
void AbstractPinnedMemHostWrite<T>::writeToDev(const T* srcPtr, bool blocking, cl::Event* event) const
{
    writeToPinnedMem(srcPtr);
    transferPinnedMemToDev(blocking, event);
}

template <typename T>
void AbstractPinnedMemHostRead<T>::readFromDev(T* dstPtr) const
{
    transferDevToPinnedMem();
    readFromPinnedMem(dstPtr);
}

template <typename T>
std::future<T*> AbstractPinnedMemHostRead<T>::readFromDevAsync(T* dstPtr) const
{
    auto transferFinishedEvent = std::unique_ptr<cl::Event>(new cl::Event);

    transferDevToPinnedMem(false, transferFinishedEvent.get());

    return std::async([this, dstPtr](std::unique_ptr<cl::Event> event)
                      {
                          event->wait();
                          readFromPinnedMem(dstPtr);
                          return dstPtr;
                      },
                      std::move(transferFinishedEvent));
}

namespace _pinned_mem_details {

// Generic pinned memory class

template <typename T>
PinnedMem<T>::PinnedMem(const cl::CommandQueue& queue)
    : _hostPtr(nullptr)
    , _q(queue)
{
}

template<typename T>
PinnedMem<T>::PinnedMem(PinnedMem&& other)
    : _hostPtr(other._hostPtr)
    , _q(std::move(other._q))
{
    other._hostPtr = nullptr;
}

template<typename T>
PinnedMem<T>& PinnedMem<T>::operator=(PinnedMem&& other)
{
    _hostPtr = other._hostPtr;
    _q = std::move(other._q);
    other._hostPtr = nullptr;

    return *this;
}

template <typename T>
T* PinnedMem<T>::hostPtr() const { return _hostPtr; }

template<typename T>
const cl::CommandQueue &PinnedMem<T>::queue() const { return _q; }

template<typename T>
void PinnedMem<T>::setHostPtr(T* hostPtr) { _hostPtr = hostPtr; }

// PinnedMemBufBase
template <typename T>
PinnedBufBase<T>::PinnedBufBase(size_t nbElements, cl_mem_flags devAccess, cl_map_flags hostAccess,
                                bool createDevBuffer, const cl::CommandQueue& queue)
    : PinnedMem<T>(queue)
    , _nbElements(nbElements)
    , _pinnedBuf(OpenCLConfig::instance().context(),
                 CL_MEM_ALLOC_HOST_PTR | devAccess,
                 sizeof(T) * nbElements)
    , _deviceBuf(createDevBuffer
                 ? cl::Buffer(OpenCLConfig::instance().context(), devAccess, sizeof(T) * nbElements)
                 : cl::Buffer())
{
    this->setHostPtr(reinterpret_cast<T*>(
                     queue.enqueueMapBuffer(_pinnedBuf, CL_TRUE, hostAccess, 0,
                                            sizeof(T) * nbElements)));
}

template<typename T>
PinnedBufBase<T>::~PinnedBufBase()
{
    if(this->hostPtr())
    {
        cl::Event e;
        this->queue().enqueueUnmapMemObject(_pinnedBuf, this->hostPtr(), nullptr, &e);
        e.wait();
    }
}

template <typename T>
const size_t& PinnedBufBase<T>::nbElements() const { return _nbElements; }

template <typename T>
const cl::Buffer& PinnedBufBase<T>::devBuffer() const { return _deviceBuf; }

template <typename T>
cl::Buffer& PinnedBufBase<T>::pinnedBuffer() { return _pinnedBuf; }

} // namespace _pinned_mem_details

} // namespace OCL
} // namespace CTL

#endif // PINNEDMEM_H
