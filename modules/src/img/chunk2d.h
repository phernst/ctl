#ifndef CHUNK2D_H
#define CHUNK2D_H

#include <QtGlobal>
#include <string>
#include <vector>

namespace CTL {
/*!
 * \class Chunk2D
 *
 * \brief The Chunk2D class provides a simple container for storage of 2D image data.
 *
 * This class is the main container used for storage of 2D image data. Typical use cases are
 * projection images and individual slices from 3D volumes.
 *
 * Internally, data is stored using an std::vector. Chunk2D is a template class that allows for
 * the storage of different data types. Most common types are:
 * \li `float` for projection data and volume slices of absorption coefficients
 * \li `unsigned short` for volume slices in Hounsfield units (HU)
 * \li `unsigned char` for segmented images (segment labels).
 *
 * By default (i.e. for most of the constructor types), memory is not allocated on creation of a
 * Chunk2D object. However, memory allocation can be enforced using allocateMemory().
 */
template <typename T>
class Chunk2D
{

public:
    struct Dimensions
    {
        uint width; //!< The width of the chunk.
        uint height; //!< The height of the chunk.

        bool operator==(const Dimensions& other) const;
        bool operator!=(const Dimensions& other) const;
        std::string info() const;
    };

    Chunk2D(const Dimensions& dimensions);
    Chunk2D(const Dimensions& dimensions, const T& fillValue);
    Chunk2D(const Dimensions& dimensions, std::vector<T>&& data);
    Chunk2D(const Dimensions& dimensions, const std::vector<T>& data);

    Chunk2D(uint width, uint height);
    Chunk2D(uint width, uint height, const T& fillValue);
    Chunk2D(uint width, uint height, std::vector<T>&& data);
    Chunk2D(uint width, uint height, const std::vector<T>& data);

    // getter methods
    size_t allocatedElements() const;
    const std::vector<T>& constData() const;
    const std::vector<T>& data() const;
    std::vector<T>& data();

    const Dimensions& dimensions() const;
    uint height() const;
    T max() const;
    T min() const;
    size_t nbElements() const;
    T* rawData();
    const T* rawData() const;
    uint width() const;

    T& operator()(uint row, uint column);
    const T& operator()(uint row, uint column) const;

    bool operator==(const Chunk2D<T>& other) const;
    bool operator!=(const Chunk2D<T>& other) const;

    Chunk2D& operator+=(const Chunk2D<T>& other);
    Chunk2D& operator-=(const Chunk2D<T>& other);
    Chunk2D& operator*=(T factor);
    Chunk2D& operator/=(T divisor);

    Chunk2D operator+(const Chunk2D<T>& other) const;
    Chunk2D operator-(const Chunk2D<T>& other) const;
    Chunk2D operator*(T factor) const;
    Chunk2D operator/(T divisor) const;

    // setter methods
    void setData(std::vector<T>&& data);
    void setData(const std::vector<T>& data);

    // other methods
    void allocateMemory();
    void fill(const T& fillValue);

protected:
    Dimensions _dim; //!< The dimensions (width x height) of the chunk.

    std::vector<T> _data; //!< The internal data of the chunk.

private:
    bool hasEqualSizeAs(const std::vector<T>& other) const;
};

/*!
 * \struct Chunk2D::Dimensions
 * \brief Dimension of Chunk2D objects.
 *
 * This struct stores the dimensions of a Chunk2D dataset. These are specified as width x height.
 */

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

/*!
 * Returns a reference to the element at position (\a row, \a column). Does not perform boundary
 * checks!
 */
template <typename T>
T& Chunk2D<T>::operator()(uint row, uint column)
{
    Q_ASSERT((row * _dim.width + column) < allocatedElements());
    return _data[row * _dim.width + column];
}

/*!
 * Returns a constant reference to the element at position (\a row, \a column). Does not perform
 * boundary checks!
 */
template <typename T>
const T& Chunk2D<T>::operator()(uint row, uint column) const
{
    Q_ASSERT((row * _dim.width + column) < allocatedElements());
    return _data[row * _dim.width + column];
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

} // namespace CTL

#include "chunk2d.tpp"

/*! \file */

#endif // CHUNK2D_H
