#ifndef COPYABLEUNIQUEPTR_H
#define COPYABLEUNIQUEPTR_H

#include <memory>

namespace CTL {
/*!
 * \class CopyableUniquePtr
 * \brief Wrapper template class that holds a `unique_ptr<T>` and provides copy construction and
 * copy assignment by using the `clone` method of type `T`.
 */
template <class T>
struct CopyableUniquePtr
{
    CopyableUniquePtr(T* pointer = nullptr) noexcept;
    CopyableUniquePtr(std::unique_ptr<T> uniquePtr) noexcept;

    CopyableUniquePtr(const CopyableUniquePtr<T>& other);
    CopyableUniquePtr(CopyableUniquePtr<T>&& other) = default;
    CopyableUniquePtr& operator=(const CopyableUniquePtr<T>& other);
    CopyableUniquePtr& operator=(CopyableUniquePtr<T>&& other) = default;
    ~CopyableUniquePtr() = default;

    T* operator->() const noexcept;
    T& operator*() const;
    explicit operator bool() const noexcept;

    T* get() const noexcept;
    void reset(T* pointer = nullptr) noexcept;

    std::unique_ptr<T> ptr; //!< the wrapped unique_ptr
};

template<class T>
CopyableUniquePtr<T>::CopyableUniquePtr(T* pointer) noexcept
    : ptr(pointer)
{
}

template<class T>
CopyableUniquePtr<T>::CopyableUniquePtr(std::unique_ptr<T> uniquePtr) noexcept
    : ptr(std::move(uniquePtr))
{
}

template<class T>
CopyableUniquePtr<T>::CopyableUniquePtr(const CopyableUniquePtr<T>& other)
    : ptr(other ? static_cast<T*>(other->clone()) : nullptr)
{
}

template<class T>
CopyableUniquePtr<T>&
CopyableUniquePtr<T>::operator=(const CopyableUniquePtr<T>& other)
{
    ptr.reset(other ? static_cast<T*>(other->clone()) : nullptr);
    return *this;
}

template<class T>
T* CopyableUniquePtr<T>::operator->() const noexcept
{
    return ptr.get();
}

template<class T>
T& CopyableUniquePtr<T>::operator*() const
{
    return *ptr;
}

template<class T>
CopyableUniquePtr<T>::operator bool() const noexcept
{
    return static_cast<bool>(ptr);
}

template<class T>
T* CopyableUniquePtr<T>::get() const noexcept
{
    return ptr.get();
}

template<class T>
void CopyableUniquePtr<T>::reset(T* pointer) noexcept
{
    ptr.reset(pointer);
}

} // namespace CTL

#endif // COPYABLEUNIQUEPTR_H
