#ifndef COPYABLEUNIQUEPTR_H
#define COPYABLEUNIQUEPTR_H

#include <memory>

namespace CTL {

/*!
 * \class CopyableUniquePtr
 * \brief Extends `std::unique_ptr<T>` with copy operations using the `clone` method of type `T`.
 */

template <class T>
class CopyableUniquePtr : private std::unique_ptr<T>
{
public:
    // ctors
    CopyableUniquePtr() = default;
    using std::unique_ptr<T>::unique_ptr;
    // copy
    CopyableUniquePtr(const CopyableUniquePtr<T>& other);
    CopyableUniquePtr& operator=(const CopyableUniquePtr<T>& other);
    // move
    CopyableUniquePtr(CopyableUniquePtr<T>&& other) = default;
    CopyableUniquePtr& operator=(CopyableUniquePtr<T>&& other) = default;
    // dtor
    ~CopyableUniquePtr() = default;

    // public access to `std::unique_ptr<T>` interface
    using std::unique_ptr<T>::get;
    using std::unique_ptr<T>::get_deleter;
    using std::unique_ptr<T>::release;
    using std::unique_ptr<T>::reset;
    using std::unique_ptr<T>::swap;
    using std::unique_ptr<T>::operator bool;
    using std::unique_ptr<T>::operator->;
    using std::unique_ptr<T>::operator*;

    // convenience methods
    bool isNull() const noexcept;
    std::unique_ptr<T>& wrapped() noexcept;
    const std::unique_ptr<T>& wrapped() const noexcept;
};

/*!
 * Constructs a new instance by calling the `clone` method of \a other or, if \a other is a
 * null pointer the object is initialized with `nullptr`.
 */
template <class T>
CopyableUniquePtr<T>::CopyableUniquePtr(const CopyableUniquePtr<T>& other)
    : std::unique_ptr<T>(other ? static_cast<T*>(other->clone()) : nullptr)
{
}

/*!
 * Copy-assignment of \a other to the current object by calling the `clone` method of \a other.
 * If \a other is a null pointer, it sets the managed object to `nullptr`.
 * Note that the old managed object (before assignment) gets destroyed.
 */
template <class T>
CopyableUniquePtr<T>& CopyableUniquePtr<T>::operator=(const CopyableUniquePtr<T>& other)
{
    this->reset(other ? static_cast<T*>(other->clone()) : nullptr);
    return *this;
}

/*!
 * Returns `true` if only a null pointer is managed and `false` otherwise.
 */
template <class T>
bool CopyableUniquePtr<T>::isNull() const noexcept
{
    return *this == nullptr;
}

/*!
 * Returns a reference to the wrapped `std::unique_ptr`.
 */
template <class T>
std::unique_ptr<T>& CopyableUniquePtr<T>::wrapped() noexcept
{
    return *this;
}

/*!
 * Returns a `const` reference to the wrapped `std::unique_ptr`.
 */
template <class T>
const std::unique_ptr<T>& CopyableUniquePtr<T>::wrapped() const noexcept
{
    return *this;
}

} // namespace CTL

#endif // COPYABLEUNIQUEPTR_H
