#ifndef PINNEDMEM_H
#define PINNEDMEM_H

#include "openclconfig.h"

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


//template<typename T>
//class PinnedMemImgReadOnly
//{
//    PinnedMemImgReadOnly(cl_mem_flags devAccess = CL_MEM_READ_ONLY);
//};


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

} // namespace _pinned_mem_details

} // namespace OCL
} // namespace CTL

#endif // PINNEDMEM_H
