#include "chunk2d.h"

#include <stdexcept>

namespace CTL {

/*!
 * Constructs a Chunk2D with dimensions of \a dimensions.
 *
 * Note that this does NOT allocate memory for storage. To do so, use allocateMemory().
 */
template <typename T>
Chunk2D<T>::Chunk2D(const Dimensions& dimensions)
    : _dim(dimensions)
{
}

/*!
 * Constructs a Chunk2D with dimensions of \a dimensions and fills it with \a fillValue.
 *
 * This constructor allocates memory for all elements.
 */
template <typename T>
Chunk2D<T>::Chunk2D(const Dimensions& dimensions, const T& fillValue)
    : Chunk2D(dimensions, std::vector<T>(size_t(dimensions.height) * dimensions.width, fillValue))
{
}

/*!
 * Constructs a Chunk2D with dimensions of \a dimensions and sets its internal data to \a data.
 * This uses the move-semantics such that the Chunk2D internal data points to the original location of
 * \a data.
 *
 * This constructor fails if the number of elements in \a data does not match the specified dimensions.
 * In this case, an std::domain_error is thrown.
 */
template <typename T>
Chunk2D<T>::Chunk2D(const Dimensions& dimensions, std::vector<T>&& data)
    : _dim(dimensions)
{
    setData(std::move(data));
}

/*!
 * Constructs a Chunk2D with dimensions of \a dimensions and sets its internal data to \a data.
 * This creates a copy of \a data. If this is not required, consider using the move-semantic alternative
 * Chunk2D(const Dimensions&, std::vector<T>&&) instead.
 *
 * This constructor fails if the number of elements in \a data does not match the specified dimensions.
 * In this case, an std::domain_error is thrown.
 */
template <typename T>
Chunk2D<T>::Chunk2D(const Dimensions& dimensions, const std::vector<T>& data)
    : _dim(dimensions)
{
    setData(data);
}

/*!
 * Constructs a Chunk2D with dimensions of (\a width x \a height).
 *
 * Note that this does NOT allocate memory for storage. To do so, use allocateMemory().
 */
template <typename T>
Chunk2D<T>::Chunk2D(uint width, uint height)
    : _dim({ width, height })
{
}

/*!
 * Constructs a Chunk2D with dimensions of (\a width x \a height) and fills it with \a fillValue.
 *
 * This constructor allocates memory for all elements.
 */
template <typename T>
Chunk2D<T>::Chunk2D(uint width, uint height, const T& fillValue)
    : Chunk2D(width, height, std::vector<T>(size_t(height) * width, fillValue))
{
}

/*!
 * Constructs a Chunk2D with dimensions of (\a width x \a height) and sets its internal data to \a data.
 * This uses the move-semantics such that the Chunk2D internal data points to the original location of
 * \a data.
 *
 * This constructor fails if the number of elements in \a data does not match the specified dimensions.
 * In this case, an std::domain_error is thrown.
 */
template <typename T>
Chunk2D<T>::Chunk2D(uint width, uint height, std::vector<T>&& data)
    : _dim({ width, height })
{
    setData(std::move(data));
}

/*!
 * Constructs a Chunk2D with dimensions of (\a width x \a height) and sets its internal data to \a data.
 * This creates a copy of \a data. If this is not required, consider using the move-semantic alternative
 * Chunk2D(const Dimensions&, std::vector<T>&&) instead.
 *
 * This constructor fails if the number of elements in \a data does not match the specified dimensions.
 * In this case, an std::domain_error is thrown.
 */
template <typename T>
Chunk2D<T>::Chunk2D(uint width, uint height, const std::vector<T>& data)
    : _dim({ width, height })
{
    setData(data);
}

template<typename T>
T Chunk2D<T>::max() const
{
    if(allocatedElements() == 0)
        return T(0);

    T tempMax = _data.front();

    for(auto el : _data)
        if(el > tempMax)
            tempMax = el;

    return tempMax;
}

template<typename T>
T Chunk2D<T>::min() const
{
    if(allocatedElements() == 0)
        return T(0);

    T tempMin = _data.front();

    for(auto el : _data)
        if(el < tempMin)
            tempMin = el;

    return tempMin;
}

/*!
 * Move-sets the internal data to \a data.
 * This uses the move-semantics such that the Chunk2D internal data points to the original location of
 * \a data.
 *
 * Throws an std::domain_error if the number of elements in \a data does not match the dimensions
 * of this chunk instance.
 */
template <typename T>
void Chunk2D<T>::setData(std::vector<T>&& data)
{
    if(!hasEqualSizeAs(data))
        throw std::domain_error("data vector has incompatible size for Chunk2D");

    _data = std::move(data);
}

/*!
 * Sets the internal data to \a data.
 * This creates a copy of \a data. If this is not required, consider using the move-semantic alternative
 * setData(std::vector<T>&&) instead.
 *
 * Throws an std::domain_error if the number of elements in \a data does not match the dimensions
 * of this chunk instance.
 */
template <typename T>
void Chunk2D<T>::setData(const std::vector<T>& data)
{
    if(!hasEqualSizeAs(data))
        throw std::domain_error("data vector has incompatible size for Chunk2D");

    _data = data;
}

/*!
 * Returns `true` if the number of elements in \a other is the same as in this instance.
 *
 * In DEBUG mode only: also performs Q_ASSERT for this equality condition.
 */
template <typename T>
bool Chunk2D<T>::hasEqualSizeAs(const std::vector<T>& other) const
{
    Q_ASSERT(nbElements() == other.size());
    return nbElements() == other.size();
}

/*!
 * Adds the data from \a other to this chunk and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this chunk instance do not match.
 */
template <typename T>
Chunk2D<T>& Chunk2D<T>::operator+=(const Chunk2D<T>& other)
{
    Q_ASSERT(_dim == other.dimensions());
    if(_dim != other.dimensions())
        throw std::domain_error("Chunk2D requires same dimensions for '+' operation:\n"
                                + dimensions().info() + " += " + other.dimensions().info());

    auto otherIt = other.constData().cbegin();
    for(auto& val : _data)
        val += *otherIt++;

    return *this;
}

/*!
 * Subtracts the data of \a other from this chunk and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this chunk instance do not match.
 */
template <typename T>
Chunk2D<T>& Chunk2D<T>::operator-=(const Chunk2D<T>& other)
{
    Q_ASSERT(_dim == other.dimensions());
    if(_dim != other.dimensions())
        throw std::domain_error("Chunk2D requires same dimensions for '-' operation:\n"
                                + dimensions().info() + " -= " + other.dimensions().info());

    auto otherIt = other.constData().cbegin();
    for(auto& val : _data)
        val -= *otherIt++;

    return *this;
}

/*!
 * Multiplies the data of this chunk element-wise by \a factor and returns a reference to this instance.
 */
template <typename T>
Chunk2D<T>& Chunk2D<T>::operator*=(T factor)
{
    for(auto& val : _data)
        val *= factor;

    return *this;
}

/*!
 * Divides the data of this chunk element-wise by \a divisor and returns a reference to this instance.
 */
template <typename T>
Chunk2D<T>& Chunk2D<T>::operator/=(T divisor)
{
    for(auto& val : _data)
        val /= divisor;

    return *this;
}

/*!
 * Adds the data from \a other to this chunk and returns the result.
 * Throws an std::domain_error if the dimensions of \a other and this chunk instance do not match.
 */
template <typename T>
Chunk2D<T> Chunk2D<T>::operator+(const Chunk2D<T>& other) const
{
    Chunk2D<T> ret(*this);
    ret += other;

    return ret;
}

/*!
 * Subtracts the data of \a other from this chunk and returns the result.
 * Throws an std::domain_error if the dimensions of \a other and this chunk instance do not match.
 */
template <typename T>
Chunk2D<T> Chunk2D<T>::operator-(const Chunk2D<T>& other) const
{
    Chunk2D<T> ret(*this);
    ret -= other;

    return ret;
}

/*!
 * Multiplies the data of this chunk element-wise by \a factor and returns the result.
 */
template <typename T>
Chunk2D<T> Chunk2D<T>::operator*(T factor) const
{
    Chunk2D<T> ret(*this);
    ret *= factor;

    return ret;
}

/*!
 * Divides the data of this chunk element-wise by \a divisor and returns the results.
 */
template <typename T>
Chunk2D<T> Chunk2D<T>::operator/(T divisor) const
{
    Chunk2D<T> ret(*this);
    ret /= divisor;

    return ret;
}

} // namespace CTL
