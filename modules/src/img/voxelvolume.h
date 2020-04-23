#ifndef CTL_VOXELVOLUME_H
#define CTL_VOXELVOLUME_H

#include "chunk2d.h"

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

        std::string info() const;
        size_t totalNbElements() const;
    };

    struct VoxelSize
    {
        float x, y, z;

        bool operator==(const VoxelSize& other) const;
        bool operator!=(const VoxelSize& other) const;

        std::string info() const;
    };
    struct Offset
    {
        float x, y, z;
    };

    // ctors (no data set)
    explicit VoxelVolume(const Dimensions& nbVoxels);
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

    VoxelVolume(const VoxelVolume&) = default;
    VoxelVolume(VoxelVolume&&) = default;
    VoxelVolume& operator=(const VoxelVolume&) = default;
    VoxelVolume& operator=(VoxelVolume&&) = default;

    // dtor (virtual)
    virtual ~VoxelVolume() = default;

    // factory
    static VoxelVolume<T> fromChunk2DStack(const std::vector<Chunk2D<T>>& stack);
    static VoxelVolume<T> ball(float radius, float voxelSize, const T& fillValue);
    static VoxelVolume<T> cube(uint nbVoxel, float voxelSize, const T& fillValue);
    static VoxelVolume<T> cylinderX(float radius, float height, float voxelSize, const T& fillValue);
    static VoxelVolume<T> cylinderY(float radius, float height, float voxelSize, const T& fillValue);
    static VoxelVolume<T> cylinderZ(float radius, float height, float voxelSize, const T& fillValue);

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

    template <class Function>
    void parallelExecution(const Function& f) const;
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

} // namespace CTL

#include "voxelvolume.tpp"

/*! \file */

#endif // CTL_VOXELVOLUME_H
