#ifndef VOXELVOLUME_H
#define VOXELVOLUME_H

#include <img/chunk2d.h>

namespace CTL {
/*!
 * \class VoxelVolume
 *
 * \brief The VoxelVolume class provides a simple container for storage of voxelized 3D volume data.
 *
 * This class is the main container used for storage of voxelized 3D volume data. The container is
 * templated. Internally, data is stored using an std::vector<T> (one-dimensional). Typical types
 * for \c T are:
 * \li `float`: for absorption coefficients \f$[\mu]=\text{mm}^{-1}\f$.
 * \li `unsigned short`: for Hounsfield units (with offset 1000).
 *
 * To specify the volume, the number of voxels in each dimensions must be defined. Physical meaning
 * is assigned to the volume by defining the dimensions of the voxels (in millimeter). The physical
 * center of the volume coincides with the origin of the world coordinate system. Optionally,
 * an offset can be specified to describe the an off-center location of the volume in space. Without
 * any offset, the center of the volume is located exactly in the origin \f$[0,0,0]\f$ of the world
 * coordinate system.
 *
 * The internal storage has row major order, i.e. consecutive values are first all voxel values in
 * *x*-direction followed by *y*-direction. At last, *z* is incremented.
 */
template <typename T>
class VoxelVolume
{
public:
    struct Dimensions
    {
        uint x, y, z;

        bool operator==(const Dimensions& other) const;
        bool operator!=(const Dimensions& other) const;
    };

    struct VoxelSize
    {
        float x, y, z;
    };
    struct Offset
    {
        float x, y, z;
    };

    // ctors (no data set)
    VoxelVolume(const Dimensions& nbVoxels);
    VoxelVolume(const Dimensions& nbVoxels, const VoxelSize& voxelSize);
    VoxelVolume(uint nbVoxelX, uint nbVoxelY, uint nbVoxelZ);
    VoxelVolume(uint nbVoxelX, uint nbVoxelY, uint nbVoxelZ, float xSize, float ySize, float zSize);

    // ctors (with data set)
    VoxelVolume(const Dimensions& nbVoxels, std::vector<T> data);
    VoxelVolume(const Dimensions& nbVoxels, const VoxelSize& voxelSize, std::vector<T> data);
    VoxelVolume(uint nbVoxelX, uint nbVoxelY, uint nbVoxelZ, std::vector<T> data);
    VoxelVolume(uint nbVoxelX,
                uint nbVoxelY,
                uint nbVoxelZ,
                float xSize,
                float ySize,
                float zSize,
                std::vector<T> data);

    // factory
    static VoxelVolume<T> fromChunk2DStack(const std::vector<Chunk2D<T>>& stack);

    // getter methods
    size_t allocatedElements() const;
    const std::vector<T>& constData() const;
    const std::vector<T>& data() const;
    std::vector<T>& data();
    const Dimensions& dimensions() const;
    bool hasData() const;
    const Dimensions& nbVoxels() const;
    const Offset& offset() const;
    T* rawData();
    const T* rawData() const;
    size_t totalVoxelCount() const;
    const VoxelSize& voxelSize() const;

    // setter methods
    void setData(std::vector<T>&& data);
    void setData(const std::vector<T>& data);
    void setVolumeOffset(const Offset& offset);
    void setVolumeOffset(float xOffset, float yOffset, float zOffset);
    void setVoxelSize(const VoxelSize& size);
    void setVoxelSize(float xSize, float ySize, float zSize);
    void setVoxelSize(float isotropicSize);

    // other methods
    void allocateMemory();
    void allocateMemory(const T& initValue);
    void fill(const T& fillValue);
    void freeMemory();
    T max() const;
    T min() const;
    VoxelVolume<T> reslicedByX(bool reverse = false) const;
    VoxelVolume<T> reslicedByY(bool reverse = false) const;
    VoxelVolume<T> reslicedByZ(bool reverse = false) const;
    Chunk2D<T> sliceX(uint slice) const;
    Chunk2D<T> sliceY(uint slice) const;
    Chunk2D<T> sliceZ(uint slice) const;
    float smallestVoxelSize() const;

    typename std::vector<T>::reference operator()(uint x, uint y, uint z);
    typename std::vector<T>::const_reference operator()(uint x, uint y, uint z) const;

    VoxelVolume<T>& operator+=(const VoxelVolume<T>& other);
    VoxelVolume<T>& operator-=(const VoxelVolume<T>& other);
    VoxelVolume<T>& operator+=(const T& additiveShift);
    VoxelVolume<T>& operator-=(const T& subtractiveShift);
    VoxelVolume<T>& operator*=(const T& factor);
    VoxelVolume<T>& operator/=(const T& divisor);

    VoxelVolume<T> operator+(const VoxelVolume<T>& other) const;
    VoxelVolume<T> operator-(const VoxelVolume<T>& other) const;
    VoxelVolume<T> operator+(const T& additiveShift) const;
    VoxelVolume<T> operator-(const T& subtractiveShift) const;
    VoxelVolume<T> operator*(const T& factor) const;
    VoxelVolume<T> operator/(const T& divisor) const;

protected:
    Dimensions _dim; //!< The dimensions of the volume.
    VoxelSize _size = { 0.0f, 0.0f, 0.0f }; //!< The size of individual voxels (in mm).
    Offset _offset = { 0.0f, 0.0f, 0.0f }; //!< The positional offset of the volume (in mm).

    std::vector<T> _data; //!< The internal data of the volume.

private:
    bool hasEqualSizeAs(const std::vector<T>& other) const;
};

/*!
 * \struct VoxelVolume::Dimensions
 * \brief Dimensions of the voxelized volume, i.e. number of voxels in each dimension.
 */

/*!
 * \struct VoxelVolume::Offset
 * \brief Offset of the voxelized volume w.r.t. the world coordinate center.
 */

/*!
 * \struct VoxelVolume::VoxelSize
 * \brief Size of individual voxels (in millimeter) in the volume.
 */

/*!
 * Returns true if all three dimensions of this instance and \a other are equal.
 */
template<typename T>
bool VoxelVolume<T>::Dimensions::operator==(const Dimensions& other) const
{
    return (x == other.x) && (y == other.y) && (z == other.z);
}

/*!
 * Returns true if any of the three dimensions of this instance and \a other differ.
 */
template<typename T>
bool VoxelVolume<T>::Dimensions::operator!=(const Dimensions& other) const
{
    return (x != other.x) || (y != other.y) || (z != other.z);
}

/*!
 * Returns the number of elements for which memory has been allocated. This is either zero if no
 * memory has been allocated (after instantiation with a non-allocating constructor) or equal to the
 * number of elements.
 *
 * Same as: constData().size().
 *
 * \sa totalVoxelCount(), allocateMemory().
 */
template <typename T>
size_t VoxelVolume<T>::allocatedElements() const
{
    return _data.size();
}

/*!
 * Enforces memory allocation. This resizes the internal std::vector to the required number of
 * elements, given by the dimensions of the chunk, i.e. width x heigth.
 *
 * As a result, allocatedElements() will return the same as totalVoxelCount().
 *
 * \sa totalVoxelCount().
 */
template <typename T>
void VoxelVolume<T>::allocateMemory()
{
    _data.resize(totalVoxelCount());
}

/*!
 * Enforces memory allocation and initilizes all elements with \a initValue.
 *
 * \sa allocateMemory(), fill().
 */
template<typename T>
void VoxelVolume<T>::allocateMemory(const T& initValue)
{
    allocateMemory();
    fill(initValue);
}

/*!
 * Fills the volume with \a fillValue. Note that this will overwrite all data stored in the volume.
 *
 * This method allocates memory for the data if it has not been allocated before.
 */
template <typename T>
void VoxelVolume<T>::fill(const T &fillValue)
{
    if(allocatedElements() != totalVoxelCount())
        allocateMemory();

    std::fill(_data.begin(), _data.end(), fillValue);
}

/*!
 * Returns a constant reference to the stored data vector.
 */
template <typename T>
const std::vector<T>& VoxelVolume<T>::constData() const
{
    return _data;
}

/*!
 * Returns a constant reference to the stored data vector.
 */
template <typename T>
const std::vector<T>& VoxelVolume<T>::data() const
{
    return _data;
}

/*!
 * Returns a reference to the stored data vector.
 */
template <typename T>
std::vector<T>& VoxelVolume<T>::data()
{
    return _data;
}

/*!
 * Returns `true` if the number of allocated elements is equal to the total number of voxels.
 * Otherwise the function returns `false`.
 * \sa totalVoxelCount(), allocatedElements()
 */
template <typename T>
bool VoxelVolume<T>::hasData() const
{
    return totalVoxelCount() == allocatedElements();
}

/*!
 * Returns the number of voxels in all three dimensions.
 *
 * For the total number of voxels in the volume, use totalVoxelCount().
 */
template <typename T>
const typename VoxelVolume<T>::Dimensions& VoxelVolume<T>::nbVoxels() const
{
    return _dim;
}

/*!
 * Returns the offset of the volume.
 */
template <typename T>
const typename VoxelVolume<T>::Offset& VoxelVolume<T>::offset() const
{
    return _offset;
}

/*!
 * Returns the pointer to the raw data.
 *
 * Same as data().data().
 */
template <typename T>
T* VoxelVolume<T>::rawData()
{
    return _data.data();
}

/*!
 * Returns the pointer to the constant raw data.
 *
 * Same as constData().data().
 */
template <typename T>
const T* VoxelVolume<T>::rawData() const
{
    return _data.data();
}

/*!
 * Returns the total number of voxels in the volume. This is the product of the voxel count in
 * all three dimensions.
 */
template <typename T>
size_t VoxelVolume<T>::totalVoxelCount() const
{
    return _dim.x * size_t(_dim.y) * _dim.z;
}

/*!
 * Returns the size of the voxels (in millimeter).
 */
template <typename T>
const typename VoxelVolume<T>::VoxelSize& VoxelVolume<T>::voxelSize() const
{
    return _size;
}

/*!
 * Returns the smallest edge length of the voxels (in millimeter).
 */
template <typename T>
float VoxelVolume<T>::smallestVoxelSize() const
{
    return qMin(qMin(_size.x,_size.y),_size.z);
}

/*!
 * Sets the offset of the volume to \a offset. This is expected to be specified in millimeter.
 */
template <typename T>
void VoxelVolume<T>::setVolumeOffset(const Offset& offset)
{
    _offset = offset;
}

/*!
 * Convenience setter. Sets the offset of the volume using the components \a xOffset, \a yOffset and
 * \a zOffset. Offset values are expected to be specified in millimeter.
 */
template <typename T>
void VoxelVolume<T>::setVolumeOffset(float xOffset, float yOffset, float zOffset)
{
    _offset = { xOffset, yOffset, zOffset };
}

/*!
 * Sets the voxel size to \a size. This is expected to be specified in millimeter.
 */
template <typename T>
void VoxelVolume<T>::setVoxelSize(const VoxelSize& size)
{
    _size = size;
}

/*!
 * Convenience setter. Sets the voxel size using the components \a xSize, \a ySize and
 * \a zSize. Dimensions are expected to be specified in millimeter.
 */
template <typename T>
void VoxelVolume<T>::setVoxelSize(float xSize, float ySize, float zSize)
{
    _size = { xSize, ySize, zSize };
}

/*!
 * Returns the number of voxels in all three dimensions.
 *
 * Same as nbVoxels().
 */
template<typename T>
const typename VoxelVolume<T>::Dimensions& VoxelVolume<T>::dimensions() const
{
    return _dim;
}

/*!
 * Move-sets the data vector to \a data. This method performs a dimensionality check and throws an
 * error if dimensions mismatch.
 */
template <typename T>
void VoxelVolume<T>::setData(std::vector<T>&& data)
{
    if(!hasEqualSizeAs(data))
        throw std::domain_error("data vector has incompatible size for VoxelVolume");

    _data = std::move(data);
}

/*!
 * Sets the data vector to \a data. This method performs a dimensionality check and throws an error
 * if dimensions mismatch.
 */
template <typename T>
void VoxelVolume<T>::setData(const std::vector<T>& data)
{
    if(!hasEqualSizeAs(data))
        throw std::domain_error("data vector has incompatible size for VoxelVolume");

    _data = data;
}

/*!
 * Checks if the number of elements in \a other match the voxel count of this instance.
 */
template <typename T>
bool VoxelVolume<T>::hasEqualSizeAs(const std::vector<T>& other) const
{
    Q_ASSERT(totalVoxelCount() == other.size());
    return totalVoxelCount() == other.size();
}


} // namespace CTL

#include "voxelvolume.tpp"

/*! \file */

#endif // VOXELVOLUME_H
