#include "imageprocessing.h"
#include "img/chunk2d.h"
#include "img/voxelvolume.h"
#include "array"

namespace CTL {
namespace imgproc {

// Implementation details
// ----------------------

// anonymous namespace
namespace {

// ## FIFO type buffer which caches number N of filter size elements ##
template <typename T, uint N>
class PipeBuffer
{
public:
    PipeBuffer(const std::array<T, N>& init) : _startPos(0), _buf(init) {}

    void addValue(const T& val)
    {
        _buf[_startPos] = val;
        ++_startPos;
        _startPos %= N;
    }

    T& operator() (uint i) { return _buf[(_startPos + i) % N]; }

private:
    uint _startPos;
    std::array<T, N> _buf;
};

// ## Derivative methods ##
template <typename T>
static void diffBuffer_null(const std::vector<T*>&)
{
}

template <typename T>
static void diffBuffer_CentralDifference(const std::vector<T*>& buffer)
{
    Q_ASSERT(buffer.size() >= 2);

    PipeBuffer<T, 3> pipe({ T(0), *buffer[0], *buffer[1] });

    const auto lastBufEl = uint(buffer.size() - 1);

    for(uint el = 1; el < lastBufEl; ++el)
    {
        pipe.addValue(*buffer[el + 1]);
        *buffer[el] = T(0.5) * (pipe(2) - pipe(0));
    }

    // set boundaries to zero
    *buffer[0] = T(0);
    *buffer[lastBufEl] = T(0);
}

// ...
// Add more methods here and
// add it to enum `Method` and following function `selectDiffFct`.

// # Method selection #
template <typename T>
using PtrToDiffFct = void (*)(const std::vector<T*>&);

template <typename T>
static PtrToDiffFct<T> selectDiffFct(Method m)
{
    PtrToDiffFct<T> ret;
    switch(m)
    {
    case CentralDifference:
        ret = &diffBuffer_CentralDifference;
        break;
    default:
        ret = &diffBuffer_null;
    }

    return ret;
}

// ## Buffer helper class that stores pointer for 1D look-ups ##
// # Base class that enables access to the buffer #
template <typename T>
class BufBase
{
public:
    explicit BufBase(size_t nbElements) : _buf(nbElements) { }

    const std::vector<T*>& get() const { return _buf; }

protected:
    template <class F>
    void fill(const F& f)
    {
        for(uint el = 0, nbEl = uint(_buf.size()); el < nbEl; ++el)
            _buf[el] = f(el);
    }

private:
    std::vector<T*> _buf;
};

// # Derived line buffer class that is specialized for each dimension #
template <typename T, uint dim>
class LineBuffer : public BufBase<T>
{
};

// # Specializations for 1st, 2nd and 3rd (x, y and z) dimension #
template <typename T>
class LineBuffer<T, 0u> : public BufBase<T> // x
{
public:
    using BufBase<T>::BufBase;

    void fillLineBuffer(Chunk2D<T>& image, uint y)
    {
        this->fill([&image, y](uint x) { return &image(x, y); });
    }

    void fillLineBuffer(VoxelVolume<T>& volume, uint y, uint z)
    {
        this->fill([&volume, y, z](uint x) { return &volume(x, y, z); });
    }
};

template <typename T>
class LineBuffer<T, 1u> : public BufBase<T> // y
{
public:
    using BufBase<T>::BufBase;

    void fillLineBuffer(Chunk2D<T>& image, uint x)
    {
        this->fill([&image, x](uint y) { return &image(x, y); });
    }

    void fillLineBuffer(VoxelVolume<T>& volume, uint x, uint z)
    {
        this->fill([&volume, x, z](uint y) { return &volume(x, y, z); });
    }
};

template <typename T>
class LineBuffer<T, 2u> : public BufBase<T> // z
{
public:
    using BufBase<T>::BufBase;

    void fillLineBuffer(VoxelVolume<T>& volume, uint x, uint y)
    {
        this->fill([&volume, x, y](uint z) { return &volume(x, y, z); });
    }
};

// ## Template implementation of diff functions ##
// # Chunk2D diff function #
template <typename T, uint dim>
void diff_impl(Chunk2D<T>& image, Method m)
{
    auto buffDiffFct = selectDiffFct<T>(m);

    uint diffDim = image.width();
    uint otherDim = image.height();
    if(dim == 1)
        std::swap(diffDim, otherDim);

    LineBuffer<T, dim> lineBuff(diffDim);
    for(uint el = 0; el < otherDim; ++el)
    {
        lineBuff.fillLineBuffer(image, el);
        buffDiffFct(lineBuff.get());
    }
}

// # VoxelVolume diff function #
template <typename T, uint dim>
void diff_impl(VoxelVolume<T>& volume, Method m)
{
    auto buffDiffFct = selectDiffFct<T>(m);

    auto volDim = volume.dimensions();
    uint diffDim, otherDim1, otherDim2;
    switch(dim)
    {
    case 0:
        diffDim = volDim.x;
        otherDim1 = volDim.y;
        otherDim2 = volDim.z;
        break;
    case 1:
        diffDim = volDim.y;
        otherDim1 = volDim.x;
        otherDim2 = volDim.z;
        break;
    case 2:
        diffDim = volDim.z;
        otherDim1 = volDim.x;
        otherDim2 = volDim.y;
        break;
    }

    LineBuffer<T, dim> lineBuff(diffDim);
    for(uint elDim1 = 0; elDim1 < otherDim1; ++elDim1)
        for(uint elDim2 = 0; elDim2 < otherDim2; ++elDim2)
        {
            lineBuff.fillLineBuffer(volume, elDim1, elDim2);
            buffDiffFct(lineBuff.get());
        }
}

} // anonymous namespace

// Interface function definitions
// ------------------------------

template <uint dim>
void diff(Chunk2D<float>& image, Method m)
{
    diff_impl<float, dim>(image, m);
}

template <uint dim>
void diff(Chunk2D<double>& image, Method m)
{
    diff_impl<double, dim>(image, m);
}

template void diff<0u>(Chunk2D<float>& image, Method m);
template void diff<1u>(Chunk2D<float>& image, Method m);
template void diff<0u>(Chunk2D<double>& image, Method m);
template void diff<1u>(Chunk2D<double>& image, Method m);

template <uint dim>
void diff(VoxelVolume<float>& volume, Method m)
{
    diff_impl<float, dim>(volume, m);
}

template <uint dim>
void diff(VoxelVolume<double>& volume, Method m)
{
    diff_impl<double, dim>(volume, m);
}

template void diff<0u>(VoxelVolume<float>& volume, Method m);
template void diff<1u>(VoxelVolume<float>& volume, Method m);
template void diff<2u>(VoxelVolume<float>& volume, Method m);
template void diff<0u>(VoxelVolume<double>& volume, Method m);
template void diff<1u>(VoxelVolume<double>& volume, Method m);
template void diff<2u>(VoxelVolume<double>& volume, Method m);

} // namespace imgproc
} // namespace CTL
