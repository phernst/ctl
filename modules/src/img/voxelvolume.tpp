#include "voxelvolume.h"
#include <cmath>

namespace CTL {

namespace details{
    template<typename T> void grindBall(VoxelVolume<T>& volume, float radius);
}

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
    const auto& firstChunk = stack.front();
    const auto& chunkDim = firstChunk.dimensions();
    const auto chunkElements = firstChunk.nbElements();
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
    for(const auto& chunk : stack)
    {
        std::copy_n(chunk.rawData(), chunkElements, retPtr);
        retPtr += chunkElements;
    }

    return ret;
}

/*!
 * Constructs a cubic voxelized volume with \a nbVoxel x \a nbVoxel x \a nbVoxel voxels (voxel
 * dimension: \a voxelSize x \a voxelSize x \a voxelSize), filled with \a fillValue.
 */
template<typename T>
VoxelVolume<T> VoxelVolume<T>::cube(uint nbVoxel, float voxelSize, const T& fillValue)
{
    const Dimensions dim{ nbVoxel, nbVoxel, nbVoxel };

    return { dim, { voxelSize, voxelSize, voxelSize },
             std::vector<T>(dim.totalNbElements(), fillValue) };
}

/*!
 * Constructs voxelized volume with voxels of isotropic dimensions \a voxelSize (in mm) and fills
 * all voxels inside a ball of radius \a radius (in mm) around the center of the volume with
 * \a fillValue. The voxels surrounding the ball are filled with zeros.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * each dimension.
 */
template<typename T>
VoxelVolume<T> VoxelVolume<T>::ball(float radius, float voxelSize, const T& fillValue)
{
    const auto nbVox = static_cast<uint>(std::ceil(2.0f * radius / voxelSize));

    const VoxelVolume<T>::Dimensions volDim{ nbVox, nbVox, nbVox };
    const VoxelVolume<T>::VoxelSize voxSize{ voxelSize, voxelSize, voxelSize };
    VoxelVolume<T> ret{ volDim, voxSize };
    ret.fill(fillValue);

    details::grindBall(ret, radius);

    return ret;
}

/*!
 * Sets an isotropic voxels size of \a isotropicSize (in mm).
 */
template<typename T>
void VoxelVolume<T>::setVoxelSize(float isotropicSize)
{
    _size = { isotropicSize, isotropicSize, isotropicSize };
}

/*!
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
    const auto voxPerSlice = size_t(_dim.x) * size_t(_dim.y);
    const auto voxPerLine = size_t(_dim.x);
    const auto lup = size_t(z) * voxPerSlice + size_t(y) * voxPerLine + size_t(x);

    Q_ASSERT(lup < _data.size());
    return _data[lup];
}

/*!
 * Returns a constant reference to the data at voxel \f$[x,y,z\f$. Does not perform boundary checks.
 */
template <typename T>
typename std::vector<T>::const_reference VoxelVolume<T>::operator()(uint x, uint y, uint z) const
{
    const auto voxPerSlice = size_t(_dim.x) * size_t(_dim.y);
    const auto voxPerLine = size_t(_dim.x);
    const auto lup = size_t(z) * voxPerSlice + size_t(y) * voxPerLine + size_t(x);

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

    const auto voxPerYZSlice = size_t(_dim.y) * size_t(_dim.z);

    std::vector<T> dataVec(voxPerYZSlice);
    // ekel loop
    auto dataIt = dataVec.begin();
    for(auto zIdx = 0u; zIdx < _dim.z; ++zIdx)
        for(auto yIdx = 0u; yIdx < _dim.y; ++yIdx, ++dataIt)
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

    const auto voxPerXZSlice = size_t(_dim.x) * size_t(_dim.z);
    const auto voxPerXYSlice = size_t(_dim.x) * size_t(_dim.y);

    std::vector<T> dataVec(voxPerXZSlice);
    // ekel loop
    const auto sliceOffset = size_t(slice) * size_t(_dim.x);
    for(auto zIdx = 0u; zIdx < _dim.z; ++zIdx)
    {
        const auto lup = sliceOffset + size_t(zIdx) * voxPerXYSlice;
        std::copy_n(_data.cbegin() + lup, _dim.x, dataVec.begin() + size_t(zIdx) * size_t(_dim.x));
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

    const auto voxPerSlice = size_t(_dim.x) * size_t(_dim.y);
    const auto lup = size_t(slice) * voxPerSlice;

    std::vector<T> dataVec(voxPerSlice);
    std::copy_n(_data.cbegin() + lup, voxPerSlice, dataVec.begin());

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
        for(auto i = _dim.x - 1u; i != static_cast<uint>(-1); --i)
            chunkStack.push_back(sliceX(i));
    else
        for(auto i = 0u; i < _dim.x; ++i)
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
        for(auto i = _dim.y - 1u; i != static_cast<uint>(-1); --i)
            chunkStack.push_back(sliceY(i));
    else
        for(auto i = 0u; i < _dim.y; ++i)
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
        for(auto i = _dim.z - 1u; i != static_cast<uint>(-1); --i)
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

    return *std::max_element(_data.cbegin(), _data.cend());
}

/*!
 * Returns the smallest value in this volume. Returns zero for empty VoxelVolume objects.
 */
template<typename T>
T VoxelVolume<T>::min() const
{
    if(allocatedElements() == 0)
        return T(0);

    return *std::min_element(_data.cbegin(), _data.cend());
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

    std::transform(_data.cbegin(), _data.cend(), other._data.cbegin(), _data.begin(),
                   [](const T& a, const T& b) { return a + b; });

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

    std::transform(_data.cbegin(), _data.cend(), other._data.cbegin(), _data.begin(),
                   [](const T& a, const T& b) { return a - b; });

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
 * Returns a string that contains the dimensions joined with " x ".
 */
template<typename T>
std::string VoxelVolume<T>::Dimensions::info() const
{
    return std::to_string(x) + " x " + std::to_string(y) + " x " + std::to_string(z);
}

/*!
 * Returns the total number of voxels in the volume.
 */
template <typename T>
size_t VoxelVolume<T>::Dimensions::totalNbElements() const
{
    return size_t(x) * size_t(y) * size_t(z);
}

/*!
 * Returns true if the voxel sizes of this instance and \a other are equal (in all three dimensions).
 */
template<typename T>
bool VoxelVolume<T>::VoxelSize::operator==(const VoxelSize& other) const
{
    return qFuzzyCompare(x, other.x) && qFuzzyCompare(y, other.y) && qFuzzyCompare(z, other.z);
}

/*!
 * Returns true if voxel sizes of this instance and \a other differ (in any of the three
 * dimensions).
 */
template<typename T>
bool VoxelVolume<T>::VoxelSize::operator!=(const VoxelSize& other) const
{
    return !qFuzzyCompare(x, other.x) || !qFuzzyCompare(y, other.y) || !qFuzzyCompare(z, other.z);
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
 * Enforces memory allocation and if the current number of allocated elements is less than the
 * number of elements in the chunk, additional copies of \a initValue are appended.
 *
 * \sa allocatedElements(), allocateMemory(), fill().
 */
template<typename T>
void VoxelVolume<T>::allocateMemory(const T& initValue)
{
    _data.resize(totalVoxelCount(), initValue);
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
    return size_t(_dim.x) * size_t(_dim.y) * size_t(_dim.z);
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
    return std::min(std::min(_size.x, _size.y), _size.z);
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

namespace details {
    template<typename T>
    void grindBall(VoxelVolume<T>& volume, float radius)
    {
        const auto nbVox = volume.dimensions().x;
        const auto center = float(nbVox - 1) / 2.0f;

        auto dist2Center = [center](float x, float y, float z)
        {
            const auto dx = x - center;
            const auto dy = y - center;
            const auto dz = z - center;
            return dx * dx + dy * dy + dz * dz;
        };

        const auto voxSize = volume.voxelSize().x;
        const auto rSquaredInVoxel = (radius / voxSize) * (radius / voxSize);

        // erase exterior space
        for(auto x = 0u; x < nbVox; ++x)
            for(auto y = 0u; y < nbVox; ++y)
                for(auto z = 0u; z < nbVox; ++z)
                    if(dist2Center(x, y, z) > rSquaredInVoxel)
                        volume(x, y, z) = 0.0f;
    }
}

} // namespace CTL
