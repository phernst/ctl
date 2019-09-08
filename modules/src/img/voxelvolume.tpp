#include "voxelvolume.h"

#include <stdexcept>

namespace CTL {

/*!
 * Constructs a voxelized volume with \a nbVoxels voxels.
 *
 * This constuctor does not allocate memory for the data. To enforce memory allocation,
 * use allocateMemory().
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(const Dimensions& nbVoxels)
    : _dim(nbVoxels)
{
}

/*!
 * Constructs a voxelized volume with [\a nbVoxelX, \a nbVoxelY, \a nbVoxelZ] voxels.
 *
 * This constuctor does not allocate memory for the data. To enforce memory allocation,
 * use allocateMemory().
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(uint nbVoxelX, uint nbVoxelY, uint nbVoxelZ)
    : _dim({ nbVoxelX, nbVoxelY, nbVoxelZ })
{
}

/*!
 * Constructs a voxelized volume with \a nbVoxels voxels. Each voxel has the (physical) dimensions
 * specified by \a voxelSize.
 *
 * This constuctor does not allocate memory for the data. To enforce memory allocation,
 * use allocateMemory().
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(const Dimensions& nbVoxels, const VoxelSize& voxelSize)
    : _dim(nbVoxels)
    , _size(voxelSize)
{
}

/*!
 * Constructs a voxelized volume with [\a nbVoxelX, \a nbVoxelY, \a nbVoxelZ] voxels. Each
 * voxel has the (physical) dimensions \a xSize x \a ySize x \a zSize (in millimeters).
 *
 * This constuctor does not allocate memory for the data. To enforce memory allocation,
 * use allocateMemory().
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(
    uint nbVoxelX, uint nbVoxelY, uint nbVoxelZ, float xSize, float ySize, float zSize)
    : _dim({ nbVoxelX, nbVoxelY, nbVoxelZ })
    , _size({ xSize, ySize, zSize })
{
}

/*!
 * Constructs a voxelized volume with \a nbVoxels voxels and sets its data to \a data.
 *
 * A dimension consistency check is performed to set the data, i.e. the number of elements
 * in \a data need to match the number of voxels in the volume. Throws std::domain_error in
 * case of mismatching dimensions.
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(const Dimensions& nbVoxels, std::vector<T> data)
    : _dim(nbVoxels)
{
    setData(std::move(data));
}

/*!
 * Constructs a voxelized volume with \a nbVoxels voxels and sets its data to \a data. Each voxel
 * has the (physical) dimensions specified by \a voxelSize.
 *
 * A dimension consistency check is performed to set the data, i.e. the number of elements
 * in \a data need to match the number of voxels in the volume. Throws std::domain_error in
 * case of mismatching dimensions.
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(const Dimensions& nbVoxels,
                            const VoxelSize& voxelSize,
                            std::vector<T> data)
    : _dim(nbVoxels)
    , _size(voxelSize)
{
    setData(std::move(data));
}

/*!
 * Constructs a voxelized volume with [\a nbVoxelX, \a nbVoxelY, \a nbVoxelZ] voxels and sets
 * its data to \a data.
 *
 * A dimension consistency check is performed to set the data, i.e. the number of elements
 * in \a data need to match the number of voxels in the volume. Throws std::domain_error in
 * case of mismatching dimensions.
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(uint nbVoxelX, uint nbVoxelY, uint nbVoxelZ, std::vector<T> data)
    : _dim({ nbVoxelX, nbVoxelY, nbVoxelZ })
{
    setData(std::move(data));
}

/*!
 * Constructs a voxelized volume with [\a nbVoxelX, \a nbVoxelY, \a nbVoxelZ] voxels and sets
 * its data to \a data. The voxels have the (physical) dimensions \a xSize x \a ySize x \a zSize
 * (in millimeters).
 *
 * A dimension consistency check is performed to set the data, i.e. the number of elements
 * in \a data need to match the number of voxels in the volume. Throws std::domain_error in
 * case of mismatching dimensions.
 */
template <typename T>
VoxelVolume<T>::VoxelVolume(uint nbVoxelX,
                            uint nbVoxelY,
                            uint nbVoxelZ,
                            float xSize,
                            float ySize,
                            float zSize,
                            std::vector<T> data)
    : _dim({ nbVoxelX, nbVoxelY, nbVoxelZ })
    , _size({ xSize, ySize, zSize })
{
    setData(std::move(data));
}

/*!
 * Constructs a voxelized volume from a stack of slices (vector of Chunk2D). All slices in \a stack
 * will be concatenated in *z*-direction.
 *
 * Note that all slices are required to have identical dimensions. Throws an std::domain_error otherwise.
 * Returns an empty VoxelVolume (all dimensions zero) when an empty stack is passed.
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::fromChunk2DStack(const std::vector<Chunk2D<T>>& stack)
{
    // check if stack is empty
    if(stack.empty())
        return { 0, 0, 0 };

    // get dim information from stack
    auto& firstChunk = stack.front();
    auto& chunkDim = firstChunk.dimensions();
    size_t chunkElements = firstChunk.nbElements();
    Dimensions volDim = { chunkDim.width, chunkDim.height, uint(stack.size()) };

    // dimension consistency check within the passed stack
    for(auto& chunk : stack)
        if(chunk.dimensions() != chunkDim)
            throw std::domain_error("Chunks in stack have different dimensions");

    // allocate volume object
    VoxelVolume<T> ret(volDim);
    ret.allocateMemory();

    // fill in data -> the data of each chunk will be copied into the volume as a z-slice
    auto retPtr = ret.rawData();
    for(auto& chunk : stack)
    {
        std::copy_n(chunk.rawData(), chunkElements, retPtr);
        retPtr += chunkElements;
    }

    return ret;
}

template<typename T>
void VoxelVolume<T>::setVoxelSize(float isotropicSize)
{
    _size = { isotropicSize, isotropicSize, isotropicSize };
}

/*
 * Deletes the data of the voxel volume.
 *
 * \sa allocateMemory()
 */
template<typename T>
void VoxelVolume<T>::freeMemory()
{
    _data.clear();
    _data.shrink_to_fit();
}

/*!
 * Returns a reference to the data at voxel \f$[x,y,z\f$. Does not perform boundary checks.
 */
template <typename T>
typename std::vector<T>::reference VoxelVolume<T>::operator()(uint x, uint y, uint z)
{
    const size_t voxPerSlice = size_t(_dim.x) * _dim.y;
    const size_t voxPerLine = size_t(_dim.x);
    size_t lup = z * voxPerSlice + y * voxPerLine + x;

    Q_ASSERT(lup < _data.size());
    return _data[lup];
}

/*!
 * Returns a constant reference to the data at voxel \f$[x,y,z\f$. Does not perform boundary checks.
 */
template <typename T>
typename std::vector<T>::const_reference VoxelVolume<T>::operator()(uint x, uint y, uint z) const
{
    const size_t voxPerSlice = size_t(_dim.x) * _dim.y;
    const size_t voxPerLine = size_t(_dim.x);
    size_t lup = z * voxPerSlice + y * voxPerLine + x;

    Q_ASSERT(lup < _data.size());
    return _data[lup];
}

/*!
 * Returns the *yz*-slice of the volume at position \f$x=\f$ \a slice as a Chunk2D.
 */
template <typename T>
Chunk2D<T> VoxelVolume<T>::sliceX(uint slice) const
{
    Q_ASSERT(slice < _dim.x);

    Chunk2D<T> ret(_dim.y, _dim.z);
    ret.allocateMemory();

    const size_t voxPerYZSlice = size_t(_dim.y) * _dim.z;

    std::vector<T> dataVec(voxPerYZSlice);
    // ekel loop
    auto dataIt = dataVec.begin();
    for(uint zIdx = 0; zIdx < _dim.z; ++zIdx)
        for(uint yIdx = 0; yIdx < _dim.y; ++yIdx, ++dataIt)
            *dataIt = (*this)(slice, yIdx, zIdx);

    ret.setData(std::move(dataVec));
    return ret;
}

/*!
 * Returns the *xz*-slice of the volume at position \f$y=\f$ \a slice as a Chunk2D.
 */
template <typename T>
Chunk2D<T> VoxelVolume<T>::sliceY(uint slice) const
{
    Q_ASSERT(slice < _dim.y);

    Chunk2D<T> ret(_dim.x, _dim.z);

    const size_t voxPerXZSlice = size_t(_dim.x) * _dim.z;
    const size_t voxPerXYSlice = size_t(_dim.x) * _dim.y;

    std::vector<T> dataVec(voxPerXZSlice);
    // ekel loop
    const uint sliceOffset = slice * _dim.x;
    for(uint zIdx = 0; zIdx < _dim.z; ++zIdx)
    {
        size_t lup = sliceOffset + zIdx * voxPerXYSlice;
        std::copy_n(_data.begin() + lup, _dim.x, dataVec.begin() + zIdx * _dim.x);
    }

    ret.setData(std::move(dataVec));
    return ret;
}

/*!
 * Returns the *xy*-slice of the volume at position \f$z=\f$ \a slice as a Chunk2D.
 */
template <typename T>
Chunk2D<T> VoxelVolume<T>::sliceZ(uint slice) const
{
    Q_ASSERT(slice < _dim.z);

    Chunk2D<T> ret(_dim.x, _dim.y);

    const size_t voxPerSlice = size_t(_dim.x) * _dim.y;
    size_t lup = slice * voxPerSlice;

    std::vector<T> dataVec(voxPerSlice);
    std::copy_n(_data.begin() + lup, voxPerSlice, dataVec.begin());

    ret.setData(std::move(dataVec));
    return ret;
}

/*!
 * Returns a copy of the volume resliced along the *x*-axis. If \a reverse is set \c true,
 * reslicing will be performed in inverse order (i.e. with descending *x* value).
 *
 * The reslicing process switches the orientation of the volume, such that the original axes:
 * [x,y,z] change to [y,z,x].
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::reslicedByX(bool reverse) const
{
    // re-slicing
    std::vector<Chunk2D<T>> chunkStack;
    chunkStack.reserve(_dim.x);
    if(reverse)
        for(int i=_dim.x-1; i>=0; --i)
            chunkStack.push_back(sliceX(i));
    else
        for(uint i=0; i<_dim.x; ++i)
            chunkStack.push_back(sliceX(i));

    return VoxelVolume<T>::fromChunk2DStack(chunkStack);
}

/*!
 * Returns a copy of the volume resliced along the *y*-axis. If \a reverse is set \c true,
 * reslicing will be performed in inverse order (i.e. with descending *y* value).
 *
 * The reslicing process switches the orientation of the volume, such that the original axes:
 * [x,y,z] change to [x,z,y].
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::reslicedByY(bool reverse) const
{
    // re-slicing
    std::vector<Chunk2D<T>> chunkStack;
    chunkStack.reserve(_dim.y);
    if(reverse)
        for(int i=_dim.y-1; i>=0; --i)
            chunkStack.push_back(sliceY(i));
    else
        for(uint i=0; i<_dim.y; ++i)
            chunkStack.push_back(sliceY(i));

    return VoxelVolume<T>::fromChunk2DStack(chunkStack);
}

/*!
 * Returns a copy of the volume resliced along the *z*-axis. If \a reverse is set \c true,
 * reslicing will be performed in inverse order (i.e. with descending *z* value).
 *
 * Since the volume is sliced in *z*-direction by default, this method will not change the
 * axes when \a reverse is \c false (default value). It simply returns an exact copy of the volume.
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::reslicedByZ(bool reverse) const
{
    // re-slicing
    std::vector<Chunk2D<T>> chunkStack;
    chunkStack.reserve(_dim.z);
    if(reverse)
        for(int i=_dim.z-1; i>=0; --i)
            chunkStack.push_back(sliceZ(i));
    else
        return *this;

    return VoxelVolume<T>::fromChunk2DStack(chunkStack);
}

/*!
 * Returns the highest value in this volume. Returns zero for empty VoxelVolume objects.
 */
template<typename T>
T VoxelVolume<T>::max() const
{
    if(allocatedElements() == 0)
        return T(0);

    T tempMax = _data.front();

    for(auto vox : _data)
        if(vox > tempMax)
            tempMax = vox;

    return tempMax;
}

/*!
 * Returns the smallest value in this volume. Returns zero for empty VoxelVolume objects.
 */
template<typename T>
T VoxelVolume<T>::min() const
{
    if(allocatedElements() == 0)
        return T(0);

    T tempMin = _data.front();

    for(auto vox : _data)
        if(vox < tempMin)
            tempMin = vox;

    return tempMin;
}

// operators
/*!
 * Adds the data from \a other to this volume and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this volume do not match.
 *
 * Note that the voxel size will be ignored (i.e. there is no consistency check). Hence, the result
 * will always have the voxel size of this instance.
 */
template <typename T>
VoxelVolume<T>& VoxelVolume<T>::operator+=(const VoxelVolume<T>& other)
{    
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("Inconsistent dimensions of VoxelVolumes in '+=' operation.");

    const auto& otherDat = other.constData();
    for(uint vox = 0; vox < totalVoxelCount(); ++vox)
        _data[vox] += otherDat[vox];

    return *this;
}

/*!
 * Subtracts the data from \a other from this volume and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this volume do not match.
 *
 * Note that the voxel size will be ignored (i.e. there is no consistency check). Hence, the result
 * will always have the voxel size of this instance.
 */
template <typename T>
VoxelVolume<T>& VoxelVolume<T>::operator-=(const VoxelVolume<T>& other)
{
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("Inconsistent dimensions of VoxelVolumes in '-=' operation.");

    const auto& otherDat = other.constData();
    for(uint vox = 0; vox < totalVoxelCount(); ++vox)
        _data[vox] -= otherDat[vox];

    return *this;
}

/*!
 * Adds \a additiveShift to all voxel values in this volume and returns a reference to this instance.
 */
template<typename T>
VoxelVolume<T>& VoxelVolume<T>::operator+=(const T& additiveShift)
{
    for(auto& vox : _data)
        vox += additiveShift;

    return *this;
}

/*!
 * Subtracts \a subtractiveShift from all voxel values in this volume and returns a reference to
 * this instance.
 */
template<typename T>
VoxelVolume<T>& VoxelVolume<T>::operator-=(const T& subtractiveShift)
{
    for(auto& vox : _data)
        vox -= subtractiveShift;

    return *this;
}

/*!
 * Multiplies all voxel values in this volume by \a factor and returns a reference to this instance.
 */
template <typename T>
VoxelVolume<T>& VoxelVolume<T>::operator*=(const T& factor)
{
    for(auto& vox : _data)
        vox *= factor;

    return *this;
}

/*!
 * Divides all voxel values in this volume by \a divisor and returns a reference to this instance.
 */
template <typename T>
VoxelVolume<T>& VoxelVolume<T>::operator/=(const T& divisor)
{
    for(auto& vox : _data)
        vox /= divisor;

    return *this;
}

/*!
 * Returns the (element-wise) sum of this volume and \a other.
 * Throws an std::domain_error if the dimensions of \a other and this volume do not match.
 *
 * Note that the voxel size will be ignored (i.e. there is no consistency check). Hence, the result
 * will always have the voxel size of this instance (i.e. left hand side operand).
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::operator+(const VoxelVolume<T>& other) const
{
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("Inconsistent dimensions of VoxelVolumes in '+' operation.");

    VoxelVolume<T> ret(*this);
    ret += other;
    return ret;
}

/*!
 * Returns the (element-wise) difference between this volume and \a other.
 * Throws an std::domain_error if the dimensions of \a other and this volume do not match.
 *
 * Note that the voxel size will be ignored (i.e. there is no consistency check). Hence, the result
 * will always have the voxel size of this instance (i.e. left hand side operand).
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::operator-(const VoxelVolume<T>& other) const
{
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("Inconsistent dimensions of VoxelVolumes in '-' operation.");

    VoxelVolume<T> ret(*this);
    ret -= other;
    return ret;
}

/*!
 * Returns a copy of this volume in which \a additiveShift has been added to all voxel values.
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::operator+(const T& additiveShift) const
{
    VoxelVolume<T> ret(*this);
    ret += additiveShift;
    return ret;
}

/*!
 * Returns a copy of this volume in which \a subtractiveShift has been subtracted from all voxel values.
 */
template <typename T>
VoxelVolume<T> VoxelVolume<T>::operator-(const T& subtractiveShift) const
{
    VoxelVolume<T> ret(*this);
    ret -= subtractiveShift;
    return ret;
}

/*!
 * Returns a copy of this volume with all its voxel values multiplied by \a factor.
 */
template<typename T>
VoxelVolume<T> VoxelVolume<T>::operator*(const T& factor) const
{
    VoxelVolume<T> ret(*this);
    ret *= factor;
    return ret;
}

/*!
 * Returns a copy of this volume with all its voxel values divided by \a divisor.
 */
template<typename T>
VoxelVolume<T> VoxelVolume<T>::operator/(const T& divisor) const
{
    VoxelVolume<T> ret(*this);
    ret /= divisor;
    return ret;
}

} // namespace CTL
