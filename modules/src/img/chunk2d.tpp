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
 * Constructs a Chunk2D with dimensions of \a dimensions and fills it with \a initValue.
 *
 * This constructor allocates memory for all elements.
 */
template <typename T>
Chunk2D<T>::Chunk2D(const Dimensions& dimensions, const T& initValue)
    : _dim(dimensions)
    , _data(size_t(dimensions.height) * dimensions.width, initValue)
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
 * Constructs a Chunk2D with dimensions of (\a width x \a height) and fills it with \a initValue.
 *
 * This constructor allocates memory for all elements.
 */
template <typename T>
Chunk2D<T>::Chunk2D(uint width, uint height, const T& initValue)
    : _dim({ width, height })
    , _data(size_t(height) * width, initValue)
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

/*!
 * Returns the maximum value in this instance.
 *
 * Returns zero if this data is empty.
 */
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

/*!
 * Returns the minimum value in this instance.
 *
 * Returns zero if this data is empty.
 */
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

/*!
 * Returns `true` if both dimensions (i.e. height and width) of \a other are identical to
 * this instance.
 *
 * \sa operator!=()
 */
template <typename T>
bool Chunk2D<T>::Dimensions::operator==(const Dimensions& other) const
{
    return (width == other.width) && (height == other.height);
}

/*!
 * Returns `true` if at least one dimension (i.e. height or width) of \a other is different from
 * the dimensions of this instance.
 *
 * \sa operator==()
 */
template <typename T>
bool Chunk2D<T>::Dimensions::operator!=(const Dimensions& other) const
{
    return (width != other.width) || (height != other.height);
}

/*!
 * Returns a string that contains the dimensions joined with " x ".
 */
template <typename T>
std::string Chunk2D<T>::Dimensions::info() const
{
    return std::to_string(width) + " x " + std::to_string(height);
}

/*!
 * Returns the total number of pixels in the chunk.
 */
template <typename T>
size_t Chunk2D<T>::Dimensions::totalNbElements() const
{
    return size_t(width()) * size_t(height());
}

/*!
 * Returns the number of elements for which memory has been allocated. This is either zero if no
 * memory has been allocated (after instantiation with a non-allocating constructor) or equal to the
 * number of elements.
 *
 * Same as: constData().size().
 *
 * \sa nbElements(), allocateMemory().
 */
template <typename T>
size_t Chunk2D<T>::allocatedElements() const
{
    return _data.size();
}

/*!
 * Returns a constant reference to the std::vector storing the data.
 */
template <typename T>
const std::vector<T>& Chunk2D<T>::constData() const
{
    return _data;
}

/*!
 * Returns a constant reference to the std::vector storing the data.
 */
template <typename T>
const std::vector<T>& Chunk2D<T>::data() const
{
    return _data;
}

/*!
 * Returns a reference to the std::vector storing the data.
 */
template <typename T>
std::vector<T>& Chunk2D<T>::data()
{
    return _data;
}

/*!
 * Returns the dimensions of the chunk.
 *
 * \sa Dimensions.
 */
template <typename T>
const typename Chunk2D<T>::Dimensions& Chunk2D<T>::dimensions() const
{
    return _dim;
}

/*!
 * Returns the height of the chunk. Same as: dimensions().height.
 *
 * \sa width().
 */
template <typename T>
uint Chunk2D<T>::height() const
{
    return _dim.height;
}

/*!
 * Returns the number of elements in the chunk. Note that these are not necessarily allocated
 * already.
 *
 * \sa allocatedElements().
 */
template <typename T>
size_t Chunk2D<T>::nbElements() const
{
    return _dim.width * size_t(_dim.height);
}

/*!
 * Returns the pointer to the raw data in the std::vector. Same as data().data().
 *
 * \sa rawData() const.
 */
template <typename T>
T* Chunk2D<T>::rawData()
{
    return _data.data();
}

/*!
 * Returns the pointer to the constant raw data in the std::vector.
 *
 * \sa rawData().
 */
template <typename T>
const T* Chunk2D<T>::rawData() const
{
    return _data.data();
}

/*!
 * Returns the width of the chunk. Same as: dimensions().width.
 *
 * \sa height().
 */
template <typename T>
uint Chunk2D<T>::width() const
{
    return _dim.width;
}

/*!
 * Fills the chunk with \a fillValue. Note that this will overwrite all data stored in the chunk.
 *
 * This method allocates memory for the data if it has not been allocated before.
 */
template <typename T>
void Chunk2D<T>::fill(const T& fillValue)
{
    if(allocatedElements() != nbElements())
        allocateMemory();

    std::fill(_data.begin(), _data.end(), fillValue);
}

/*
 * Deletes the data of the chunk.
 *
 * \sa allocateMemory()
 */
template<typename T>
void Chunk2D<T>::freeMemory()
{
    _data.clear();
    _data.shrink_to_fit();
}

/*!
 * Returns a reference to the element at position (\a x, \a y) or (column, row).
 * Does not perform boundary checks!
 */
template <typename T>
typename std::vector<T>::reference Chunk2D<T>::operator()(uint x, uint y)
{
    Q_ASSERT((y * _dim.width + x) < allocatedElements());
    return _data[y * _dim.width + x];
}

/*!
 * Returns a constant reference to the element at position (\a x, \a y) or (column, row).
 * Does not perform boundary checks!
 */
template <typename T>
typename std::vector<T>::const_reference Chunk2D<T>::operator()(uint x, uint y) const
{
    Q_ASSERT((y * _dim.width + x) < allocatedElements());
    return _data[y * _dim.width + x];
}

/*!
 * Returns a true if the dimensions and data of \a other are equal to those of this chunk.
 */
template <typename T>
bool Chunk2D<T>::operator==(const Chunk2D<T>& other) const
{
    return (_dim == other._dim) && (_data == other._data);
}

/*!
 * Returns a true if either the dimensions or the data of \a other differ from those of this chunk.
 */
template <typename T>
bool Chunk2D<T>::operator!=(const Chunk2D<T>& other) const
{
    return (_dim != other._dim) || (_data != other._data);
}

/*!
 * Enforces memory allocation. This resizes the internal std::vector to the required number of
 * elements, given by the dimensions of the chunk, i.e. width x heigth.
 * As a result, allocatedElements() will return the same as nbElements().
 *
 * \sa nbElements().
 */
template <typename T>
void Chunk2D<T>::allocateMemory()
{
    _data.resize(nbElements());
}

/*!
 * Enforces memory allocation and if the current number of allocated elements is less than the
 * number of elements in the chunk, additional copies of \a initValue are appended.
 *
 * \sa allocatedElements(), allocateMemory(), fill().
 */
template <typename T>
void Chunk2D<T>::allocateMemory(const T& initValue)
{
    _data.resize(nbElements(), initValue);
}

} // namespace CTL
