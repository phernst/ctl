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
class CopyableUniquePtr
{
public:
    CopyableUniquePtr(T* pointer = nullptr) noexcept;
    CopyableUniquePtr(std::unique_ptr<T> uniquePtr) noexcept;

    CopyableUniquePtr(const CopyableUniquePtr<T>& other);
    CopyableUniquePtr(CopyableUniquePtr<T>&& other) = default;
    CopyableUniquePtr& operator=(const CopyableUniquePtr<T>& other);
    CopyableUniquePtr& operator=(CopyableUniquePtr<T>&& other) = default;
    ~CopyableUniquePtr() = default;

    T* get() const noexcept;
    bool isNull() const noexcept;
    void reset(T* pointer = nullptr) noexcept;
    std::unique_ptr<T>& wrapped() noexcept;
    const std::unique_ptr<T>& wrapped() const noexcept;

    T* operator->() const noexcept;
    T& operator*() const;
    explicit operator bool() const noexcept;

private:
    std::unique_ptr<T> _uPtr; //!< the wrapped unique_ptr
};

template <class T>
CopyableUniquePtr<T>::CopyableUniquePtr(T* pointer) noexcept
    : _uPtr(pointer)
{
}

template <class T>
CopyableUniquePtr<T>::CopyableUniquePtr(std::unique_ptr<T> uniquePtr) noexcept
    : _uPtr(std::move(uniquePtr))
{
}

template <class T>
CopyableUniquePtr<T>::CopyableUniquePtr(const CopyableUniquePtr<T>& other)
    : _uPtr(other ? static_cast<T*>(other->clone()) : nullptr)
{
}

template <class T>
CopyableUniquePtr<T>& CopyableUniquePtr<T>::operator=(const CopyableUniquePtr<T>& other)
{
    _uPtr.reset(other ? static_cast<T*>(other->clone()) : nullptr);
    return *this;
}

template <class T>
bool CopyableUniquePtr<T>::isNull() const noexcept
{
    return _uPtr == nullptr;
}

template <class T>
T* CopyableUniquePtr<T>::get() const noexcept
{
    return _uPtr.get();
}

template <class T>
void CopyableUniquePtr<T>::reset(T* pointer) noexcept
{
    _uPtr.reset(pointer);
}

template <class T>
std::unique_ptr<T>& CopyableUniquePtr<T>::wrapped() noexcept
{
    return _uPtr;
}

template <class T>
const std::unique_ptr<T>& CopyableUniquePtr<T>::wrapped() const noexcept
{
    return _uPtr;
}

template <class T>
T* CopyableUniquePtr<T>::operator->() const noexcept
{
    return _uPtr.get();
}

template <class T>
T& CopyableUniquePtr<T>::operator*() const
{
    return *_uPtr;
}

template <class T>
CopyableUniquePtr<T>::operator bool() const noexcept
{
    return static_cast<bool>(_uPtr);
}

} // namespace CTL

#endif // COPYABLEUNIQUEPTR_H
