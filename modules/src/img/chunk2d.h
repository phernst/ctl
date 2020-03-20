#ifndef CTL_CHUNK2D_H
#define CTL_CHUNK2D_H

#include <QtGlobal>
#include <algorithm>
#include <stdexcept>
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
        size_t totalNbElements() const;
    };

    explicit Chunk2D(const Dimensions& dimensions);
    Chunk2D(const Dimensions& dimensions, const T& initValue);
    Chunk2D(const Dimensions& dimensions, std::vector<T>&& data);
    Chunk2D(const Dimensions& dimensions, const std::vector<T>& data);

    Chunk2D(uint width, uint height);
    Chunk2D(uint width, uint height, const T& initValue);
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

    typename std::vector<T>::reference operator()(uint x, uint y);
    typename std::vector<T>::const_reference operator()(uint x, uint y) const;

    bool operator==(const Chunk2D<T>& other) const;
    bool operator!=(const Chunk2D<T>& other) const;

    Chunk2D& operator+=(const Chunk2D<T>& other);
    Chunk2D& operator-=(const Chunk2D<T>& other);
    Chunk2D& operator*=(const T& factor);
    Chunk2D& operator/=(const T& divisor);

    Chunk2D operator+(const Chunk2D<T>& other) const;
    Chunk2D operator-(const Chunk2D<T>& other) const;
    Chunk2D operator*(const T& factor) const;
    Chunk2D operator/(const T& divisor) const;

    // setter methods
    void setData(std::vector<T>&& data);
    void setData(const std::vector<T>& data);

    // other methods
    void allocateMemory();
    void allocateMemory(const T& initValue);
    void fill(const T& fillValue);
    void freeMemory();

protected:
    std::vector<T> _data; //!< The internal data of the chunk.
    Dimensions _dim; //!< The dimensions (width x height) of the chunk.

private:
    bool hasEqualSizeAs(const std::vector<T>& other) const;
};

/*!
 * \struct Chunk2D::Dimensions
 * \brief Dimension of Chunk2D objects.
 *
 * This struct stores the dimensions of a Chunk2D dataset. These are specified as width x height.
 */

} // namespace CTL

#include "chunk2d.tpp"

/*! \file */

#endif // CTL_CHUNK2D_H
