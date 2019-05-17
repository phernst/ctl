#ifndef PINNEDMEM_H
#define PINNEDMEM_H

#include "openclconfig.h"
#include <array>

namespace CTL {
namespace OCL {

namespace _pinned_mem_details {

template<typename T, cl_mem_flags devAccess>
class PinnedMemBufBase
{
public:
    PinnedMemBufBase(size_t nbElements);

    const size_t& nbElements() const;
    cl::Buffer& devBuffer();

protected:
    cl::Buffer& pinnedBuffer();

private:
    size_t _nbElements;
    cl::Buffer _pinnedBuf;
    cl::Buffer _deviceBuf;
};

template<typename T, cl_mem_flags devAccess>
class PinnedMemImag3DBase
{
public:
    PinnedMemImag3DBase(size_t xDim, size_t yDim, size_t zDim);

    const std::array<size_t, 3>& nbElements() const;
    cl::Image3D& devImage();

protected:
    cl::Image3D& pinnedImage();

private:
    std::array<size_t, 3> _nbElements;
    cl::Image3D _pinnedImg;
    cl::Image3D _deviceImg;
};


} // namespace _pinnedMemDetails

template<typename T>
class PinnedMemBufReadOnly : public _pinned_mem_details::PinnedMemBufBase<T, CL_MEM_READ_ONLY>
{
public:
    PinnedMemBufReadOnly(size_t nbElements, const cl::CommandQueue& queue);

    // access to pinned memory
    T* hostPtr() const;

    // copy from pinned memory to device (all elements)
    void copyPinnedMemToDev(bool blocking = true, cl::Event* event = nullptr);
    // copy from srcPtr to pinned memory (all elements)
    void copyToPinnedMem(const T* srcPtr);

    // first copy from srcPtr to pinned memory and then to device (all elements)
    void copyToDev(const T* srcPtr, bool blocking = true, cl::Event* event = nullptr);

private:
    cl::CommandQueue _q; //!< device specific command queue
    T* _hostPtr; //!< pointer to pinned memory
};


template<typename T>
class PinnedMemImg3DReadOnly : public _pinned_mem_details::PinnedMemImag3DBase<T, CL_MEM_READ_ONLY>
{
    PinnedMemImg3DReadOnly(size_t xDim, size_t yDim, size_t zDim, const cl::CommandQueue& queue);

    // access to pinned memory
    T* hostPtr() const;

    // copy from pinned memory to device (all elements)
    void copyPinnedMemToDev(bool blocking = true, cl::Event* event = nullptr);
    // copy from srcPtr to pinned memory (all elements)
    void copyToPinnedMem(const T* srcPtr);

    // first copy from srcPtr to pinned memory and then to device (all elements)
    void copyToDev(const T* srcPtr, bool blocking = true, cl::Event* event = nullptr);

private:
    cl::CommandQueue _q; //!< device specific command queue
    T* _hostPtr; //!< pointer to pinned memory

    static cl::size_t<3> zeros();
    cl::size_t<3> dimensions() const;
};


// ### IMPLEMENTATION ###

template<typename T>
PinnedMemBufReadOnly<T>::PinnedMemBufReadOnly(size_t nbElements, const cl::CommandQueue &queue)
    : _pinned_mem_details::PinnedMemBufBase<T, CL_MEM_READ_ONLY>(nbElements)
    , _q(queue)
    , _hostPtr(queue.enqueueMapBuffer(pinnedBuffer(), CL_TRUE, CL_MAP_WRITE, 0,
                                      sizeof(T) * this->nbElements()))
{
}

template<typename T>
T *PinnedMemBufReadOnly<T>::hostPtr() const { return _hostPtr; }

template<typename T>
void PinnedMemBufReadOnly<T>::copyPinnedMemToDev(bool blocking, cl::Event* event)
{
    _q.enqueueWriteBuffer(devBuffer(), blocking ? CL_TRUE : CL_FALSE, 0,
                          sizeof(T) * this->nbElements(), _hostPtr, nullptr, event);
}

template<typename T>
void PinnedMemBufReadOnly<T>::copyToPinnedMem(const T* srcPtr)
{
    std::copy_n(srcPtr, srcPtr + nbElements(), _hostPtr);
}

template<typename T>
void PinnedMemBufReadOnly<T>::copyToDev(const T* srcPtr, bool blocking, cl::Event *event)
{
    copyToPinnedMem(srcPtr);
    copyPinnedMemToDev(blocking, event);
}

template<typename T>
PinnedMemImg3DReadOnly<T>::PinnedMemImg3DReadOnly(size_t xDim, size_t yDim, size_t zDim,
                                                  const cl::CommandQueue& queue)
    : _pinned_mem_details::PinnedMemImag3DBase<T, CL_MEM_READ_ONLY>(xDim, yDim, zDim)
    , _q(queue)
{
    size_t rowPitch, slicePitch;
    _hostPtr = queue.enqueueMapImage(pinnedImage(), CL_TRUE, CL_MAP_WRITE, zeros(), dimensions(),
                                     rowPitch, slicePitch);
}

template<typename T>
T* PinnedMemImg3DReadOnly<T>::hostPtr() const
{
    return _hostPtr;
}

template<typename T>
void PinnedMemImg3DReadOnly<T>::copyPinnedMemToDev(bool blocking, cl::Event* event)
{
    _q.enqueueWriteImage(devImage(), blocking ? CL_TRUE : CL_FALSE, zeros(), dimensions(), 0, 0,
                         _hostPtr, nullptr, event);
}

template<typename T>
void PinnedMemImg3DReadOnly<T>::copyToPinnedMem(const T* srcPtr)
{
    std::copy_n(srcPtr, srcPtr + nbElements()[0] * nbElements()[1] * nbElements()[2], _hostPtr);
}

template<typename T>
void PinnedMemImg3DReadOnly<T>::copyToDev(const T* srcPtr, bool blocking, cl::Event* event)
{
    copyToPinnedMem(srcPtr);
    copyPinnedMemToDev(blocking, event);
}

template<typename T>
cl::size_t<3> PinnedMemImg3DReadOnly<T>::zeros()
{
    static cl::size_t<3> zeros;
    zeros[0] = 0;
    zeros[1] = 0;
    zeros[2] = 0;
    return zeros;
}

template<typename T>
cl::size_t<3> PinnedMemImg3DReadOnly<T>::dimensions() const
{
    cl::size_t<3> dims;
    dims[0] = nbElements()[0];
    dims[1] = nbElements()[1];
    dims[2] = nbElements()[2];
    return dims;
}


// ### DETAILS ###

namespace _pinned_mem_details {

template<typename T, cl_mem_flags devAccess>
PinnedMemBufBase<T, devAccess>::PinnedMemBufBase(size_t nbElements)
    : _nbElements(nbElements)
    , _pinnedBuf(OpenCLConfig::instance().context(), CL_MEM_ALLOC_HOST_PTR | devAccess, sizeof(T) * nbElements)
    , _deviceBuf(OpenCLConfig::instance().context(), devAccess, sizeof(T) * nbElements)
{
}

template<typename T, cl_mem_flags devAccess>
const size_t& PinnedMemBufBase<T, devAccess>::nbElements() const
{
    return _nbElements;
}

template<typename T, cl_mem_flags devAccess>
cl::Buffer& PinnedMemBufBase<T, devAccess>::devBuffer()
{
    return _deviceBuf;
}

template<typename T, cl_mem_flags devAccess>
cl::Buffer& PinnedMemBufBase<T, devAccess>::pinnedBuffer()
{
    return _pinnedBuf;
}

template<typename T, cl_mem_flags devAccess>
PinnedMemImag3DBase<T, devAccess>::PinnedMemImag3DBase(size_t xDim, size_t yDim, size_t zDim)
    : _nbElements{ xDim, yDim, zDim }
{
}

template<typename T, cl_mem_flags devAccess>
const std::array<size_t, 3>& PinnedMemImag3DBase<T, devAccess>::nbElements() const
{
    return _nbElements;
}

template<typename T, cl_mem_flags devAccess>
cl::Image3D& PinnedMemImag3DBase<T, devAccess>::devImage()
{
    return _deviceImg;
}

template<typename T, cl_mem_flags devAccess>
cl::Image3D& PinnedMemImag3DBase<T, devAccess>::pinnedImage()
{
    return _pinnedImg;
}

} // namespace _pinned_mem_details

} // namespace OCL
} // namespace CTL

#endif // PINNEDMEM_H
