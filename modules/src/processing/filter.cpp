#include "diff.h"
#include "filter.h"
#include "img/chunk2d.h"
#include "img/voxelvolume.h"
#include <array>
#include <cmath>

/*!
 * This cpp implements the partial `diff` and one-dimensional `filter` functions declared in
 * "diff.h" and "filter.h".
 */

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
    explicit PipeBuffer(const std::vector<T*>& init)
    {
        // initialize N-1 elements with init by using an offset of 1 in the array, i.e.
        // array = { 0, *init[0], *init[1], *init[2], ... }
        for(uint n = 0; n < N - 1; ++n)
            _buf[n + 1] = *init[n];
    }

    void addValue(const T& val)
    {
        _buf[_startPos] = val;
        ++_startPos;
        _startPos %= N;
    }

    const T& operator() (uint i) const { return _buf[(_startPos + i) % N]; }

private:
    uint _startPos = 0;
    std::array<T, N> _buf;
};

// ## Generic function for numerical derivate using a certain function f on a PipeBuffer ###
template <typename T, uint filterSize>
using ResValFromPipeBuf = T (*)(const PipeBuffer<T, filterSize>&);

template <typename T, uint filterSize>
void meta_filt(const std::vector<T*>& buffer, ResValFromPipeBuf<T, filterSize> f)
{
    Q_ASSERT(buffer.size() >= filterSize - 1);

    PipeBuffer<T, filterSize> pipe(buffer);

    // number of filter elements on the left and right hand side (equal for odd filter size)
    const auto nbRightFilterEl = filterSize / 2;
    const auto nbLeftFilterEl = (filterSize - 1) / 2;
    const auto firstUndefEl = uint(buffer.size() - nbRightFilterEl);

    for(uint el = nbLeftFilterEl; el < firstUndefEl; ++el)
    {
        pipe.addValue(*buffer[el + nbRightFilterEl]);
        *buffer[el] = f(pipe);
    }

    // set boundaries to zero
    for(uint b = 0; b < nbLeftFilterEl; ++b)
        *buffer[b] = T(0);
    for(uint b = 0; b < nbRightFilterEl; ++b)
        *buffer[firstUndefEl + b] = T(0);
}

// ## Derivative/Filter methods ##
// Derivative/Filter methods need to have the following signature:
// template <typename T> void NAME (const std::vector<T*>&)
// The input argument is a vector providing pointers to all elements along the dimension of
// differentiation/filtering. Compute the derivative/filtered values and overwrite them using the
// provided pointers. This can be easily done by calling `meta_filt` and passing the buffer as well
// as a pointer to a function that performs a derivative/filter based on the values provided by a
// PipeBuffer<T,filterSize>.
// This means, ony the formula for a single value of the derivative/filter based on the adjacent elements
// (the number of neighbors is given by the `filterSize` template argument) needs to be provided.

// Derivatives
template <typename T>
void filterBuffer_null(const std::vector<T*>&)
{
}

template <typename T>
void diffBuffer_CentralDifference(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 3;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return T(0.5) * (pipe(2) - pipe(0));
    });
}

template <typename T>
void diffBuffer_DifferenceToNext(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 2;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return pipe(1) - pipe(0);
    });
}

template <typename T>
void diffBuffer_SavitzkyGolay5(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 5;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return T(0.1) * (- T(2) * pipe(0)
                         -        pipe(1)

                         +        pipe(3)
                         + T(2) * pipe(4));
    });
}

template <typename T>
void diffBuffer_SavitzkyGolay7(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 7;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return T(1)/T(28) * (- T(3) * pipe(0)
                             - T(2) * pipe(1)
                             -        pipe(2)

                             +        pipe(4)
                             + T(2) * pipe(5)
                             + T(3) * pipe(6));
    });
}

template <typename T>
void diffBuffer_SpectralGauss3(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 15;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return  - T(0.00148810) * pipe(0)
                + T(0.00238095) * pipe(1)
                - T(0.00416667) * pipe(2)
                + T(0.00833333) * pipe(3)
                - T(0.02083333) * pipe(4)
                + T(0.08333333) * pipe(5)
                + T(0.37500000) * pipe(6)

                - T(0.37500000) * pipe(8)
                - T(0.08333333) * pipe(9)
                + T(0.02083333) * pipe(10)
                - T(0.00833333) * pipe(11)
                + T(0.00416667) * pipe(12)
                - T(0.00238095) * pipe(13)
                + T(0.00148810) * pipe(14);
    });
}

template <typename T>
void diffBuffer_SpectralGauss5(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 7;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return  + T(0.01250000) * pipe(0)
                + T(0.13020833) * pipe(1)
                + T(0.20833333) * pipe(2)

                - T(0.20833333) * pipe(4)
                - T(0.13020833) * pipe(5)
                - T(0.01250000) * pipe(6);
    });
}

template <typename T>
void diffBuffer_SpectralGauss7(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 7;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return  + T(0.03828125) * pipe(0)
                + T(0.12031250) * pipe(1)
                + T(0.13671875) * pipe(2)

                - T(0.13671875) * pipe(4)
                - T(0.12031250) * pipe(5)
                - T(0.03828125) * pipe(6);
    });
}

template <typename T>
void diffBuffer_SpectralGauss9(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 9;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return  + T(0.01061663) * pipe(0)
                + T(0.04977679) * pipe(1)
                + T(0.10390625) * pipe(2)
                + T(0.09843750) * pipe(3)

                - T(0.09843750) * pipe(5)
                - T(0.10390625) * pipe(6)
                - T(0.04977679) * pipe(7)
                - T(0.01061663) * pipe(8);
    });
}

// Generic Filters
template <typename T>
void filterBuffer_Gauss3(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 3;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return T(0.25) * pipe(0) + T(0.5) * pipe(1) + T(0.25) * pipe(2);
    });
}

template <typename T>
void filterBuffer_Average3(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 3;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return T(1.0/3.0) * pipe(0) + T(1.0/3.0) * pipe(1) + T(1.0/3.0) * pipe(2);
    });
}

template <typename T>
void filterBuffer_Median3(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 3;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        if(pipe(0) > pipe(1))
        {
            if(pipe(0) < pipe(2))
                return pipe(0);
            else
                return pipe(1) < pipe(2) ? pipe(2) : pipe(1);
        }
        else
        {
            if(pipe(1) < pipe(2))
                return pipe(1);
            else
                return pipe(0) < pipe(2) ? pipe(2) : pipe(0);
        }
    });
}

template <typename T>
void filterBuffer_MedianAbs3(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 3;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        if(std::fabs(pipe(0)) > std::fabs(pipe(1)))
        {
            if(std::fabs(pipe(0)) < std::fabs(pipe(2)))
                return pipe(0);
            else
                return std::fabs(pipe(1)) < std::fabs(pipe(2)) ? pipe(2) : pipe(1);
        }
        else
        {
            if(std::fabs(pipe(1)) < std::fabs(pipe(2)))
                return pipe(1);
            else
                return std::fabs(pipe(0)) < std::fabs(pipe(2)) ? pipe(2) : pipe(0);
        }
    });
}

template <typename T>
void filterBuffer_MaxAbs3(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 3;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        if(std::fabs(pipe(0)) > std::fabs(pipe(1)))
        {
            if(std::fabs(pipe(0)) > std::fabs(pipe(2)))
                return pipe(0);
        }
        else
        {
            if(std::fabs(pipe(1)) > std::fabs(pipe(2)))
                return pipe(1);
        }

        return pipe(2);
    });
}

template <typename T>
void filterBuffer_RamLak43(const std::vector<T*>& buffer)
{
    constexpr uint filterSize = 43;
    meta_filt<T, filterSize>(buffer, [](const PipeBuffer<T, filterSize>& pipe)
    {
        return - T(0.0002297532508897) * pipe(0)

               - T(0.0002806680987322) * pipe(2)

               - T(0.0003505923309423) * pipe(4)

               - T(0.0004503163717437) * pipe(6)

               - T(0.0005995336310198) * pipe(8)

               - T(0.0008373651540689) * pipe(10)

               - T(0.0012508788103992) * pipe(12)

               - T(0.0020677792580069) * pipe(14)

               - T(0.0040528473456935) * pipe(16)

               - T(0.0112579092935931) * pipe(18)

               - T(0.1013211836423378) * pipe(20)
               + T(0.2500000000000000) * pipe(21)
               - T(0.1013211836423378) * pipe(22)

               - T(0.0112579092935931) * pipe(24)

               - T(0.0040528473456935) * pipe(26)

               - T(0.0020677792580069) * pipe(28)

               - T(0.0012508788103992) * pipe(30)

               - T(0.0008373651540689) * pipe(32)

               - T(0.0005995336310198) * pipe(34)

               - T(0.0004503163717437) * pipe(36)

               - T(0.0003505923309423) * pipe(38)

               - T(0.0002806680987322) * pipe(40)

               - T(0.0002297532508897) * pipe(42);
    });
}

// ...
// Add more methods here and
// add it to enum `[Diff]/[Filt]Method` and following function `selectFilterFct`.

// # Method selection #
template <typename T>
using PtrToFilterFct = void (*)(const std::vector<T*>&);

template <typename T>
PtrToFilterFct<T> selectFilterFct(int m)
{
    switch(m)
    {
    // Derivatives
    case DiffMethod::CentralDifference:
        return &diffBuffer_CentralDifference;
    case DiffMethod::DifferenceToNext:
        return &diffBuffer_DifferenceToNext;
    case DiffMethod::SavitzkyGolay5:
        return &diffBuffer_SavitzkyGolay5;
    case DiffMethod::SavitzkyGolay7:
        return &diffBuffer_SavitzkyGolay7;
    case DiffMethod::SpectralGauss3:
        return &diffBuffer_SpectralGauss3;
    case DiffMethod::SpectralGauss5:
        return &diffBuffer_SpectralGauss5;
    case DiffMethod::SpectralGauss7:
        return &diffBuffer_SpectralGauss7;
    case DiffMethod::SpectralGauss9:
        return &diffBuffer_SpectralGauss9;

    // Generic Filters
    case FiltMethod::Gauss3:
        return &filterBuffer_Gauss3;
    case FiltMethod::Average3:
        return &filterBuffer_Average3;
    case FiltMethod::Median3:
        return &filterBuffer_Median3;
    case FiltMethod::MedianAbs3:
        return &filterBuffer_MedianAbs3;
    case FiltMethod::MaxAbs3:
        return &filterBuffer_MaxAbs3;
    case FiltMethod::RamLak43:
        return &filterBuffer_RamLak43;
    }
    return &filterBuffer_null;
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

// ## Template implementation of diff/filter functions ##
// # Chunk2D diff/filter function #
template <typename T, uint dim>
void filter_impl(Chunk2D<T>& image, int m)
{
    auto buffFilterFct = selectFilterFct<T>(m);

    uint filterDim = image.width();
    uint otherDim = image.height();
    if(dim == 1)
        std::swap(filterDim, otherDim);

    LineBuffer<T, dim> lineBuff(filterDim);
    for(uint el = 0; el < otherDim; ++el)
    {
        lineBuff.fillLineBuffer(image, el);
        buffFilterFct(lineBuff.get());
    }
}

// # VoxelVolume diff/filter function #
template <typename T, uint dim>
void filter_impl(VoxelVolume<T>& volume, int m)
{
    auto buffFilterFct = selectFilterFct<T>(m);

    auto volDim = volume.dimensions();
    uint filterDim, otherDim1, otherDim2;
    switch(dim)
    {
    case 0:
        filterDim = volDim.x;
        otherDim1 = volDim.y;
        otherDim2 = volDim.z;
        break;
    case 1:
        filterDim = volDim.y;
        otherDim1 = volDim.x;
        otherDim2 = volDim.z;
        break;
    case 2:
        filterDim = volDim.z;
        otherDim1 = volDim.x;
        otherDim2 = volDim.y;
        break;
    }

    LineBuffer<T, dim> lineBuff(filterDim);
    for(uint elDim1 = 0; elDim1 < otherDim1; ++elDim1)
        for(uint elDim2 = 0; elDim2 < otherDim2; ++elDim2)
        {
            lineBuff.fillLineBuffer(volume, elDim1, elDim2);
            buffFilterFct(lineBuff.get());
        }
}

} // anonymous namespace

// Interface function definitions
// ------------------------------
/*!
 * Differentiates the data in \a image along the dimension \a dim using the differentiation method
 * \a m.
 */
template <uint dim>
void diff(Chunk2D<float>& image, DiffMethod m)
{
    filter_impl<float, dim>(image, m);
}

/*!
 * Filters the data in \a image along the dimension \a dim using the filter method \a m.
 */
template <uint dim>
void filter(Chunk2D<float>& image, FiltMethod m)
{
    filter_impl<float, dim>(image, m);
}

/*!
 * Differentiates the data in \a image along the dimension \a dim using the differentiation method
 * \a m.
 */
template <uint dim>
void diff(Chunk2D<double>& image, DiffMethod m)
{
    filter_impl<double, dim>(image, m);
}

/*!
 * Filters the data in \a image along the dimension \a dim using the filter method \a m.
 */
template <uint dim>
void filter(Chunk2D<double>& image, FiltMethod m)
{
    filter_impl<double, dim>(image, m);
}

template void diff<0u>(Chunk2D<float>& image, DiffMethod m);
template void diff<1u>(Chunk2D<float>& image, DiffMethod m);
template void diff<0u>(Chunk2D<double>& image, DiffMethod m);
template void diff<1u>(Chunk2D<double>& image, DiffMethod m);

template void filter<0u>(Chunk2D<float>& image, FiltMethod m);
template void filter<1u>(Chunk2D<float>& image, FiltMethod m);
template void filter<0u>(Chunk2D<double>& image, FiltMethod m);
template void filter<1u>(Chunk2D<double>& image, FiltMethod m);

/*!
 * Differentiates the data in \a volume along the dimension \a dim using the differentiation method
 * \a m.
 */
template <uint dim>
void diff(VoxelVolume<float>& volume, DiffMethod m)
{
    filter_impl<float, dim>(volume, m);
}

/*!
 * Filters the data in \a volume along the dimension \a dim using the filter method \a m.
 */
template <uint dim>
void filter(VoxelVolume<float>& volume, FiltMethod m)
{
    filter_impl<float, dim>(volume, m);
}

/*!
 * Differentiates the data in \a volume along the dimension \a dim using the differentiation method
 * \a m.
 */
template <uint dim>
void diff(VoxelVolume<double>& volume, DiffMethod m)
{
    filter_impl<double, dim>(volume, m);
}

/*!
 * Filters the data in \a volume along the dimension \a dim using the filter method \a m.
 */
template <uint dim>
void filter(VoxelVolume<double>& volume, FiltMethod m)
{
    filter_impl<double, dim>(volume, m);
}

template void diff<0u>(VoxelVolume<float>& volume, DiffMethod m);
template void diff<1u>(VoxelVolume<float>& volume, DiffMethod m);
template void diff<2u>(VoxelVolume<float>& volume, DiffMethod m);
template void diff<0u>(VoxelVolume<double>& volume, DiffMethod m);
template void diff<1u>(VoxelVolume<double>& volume, DiffMethod m);
template void diff<2u>(VoxelVolume<double>& volume, DiffMethod m);

template void filter<0u>(VoxelVolume<float>& volume, FiltMethod m);
template void filter<1u>(VoxelVolume<float>& volume, FiltMethod m);
template void filter<2u>(VoxelVolume<float>& volume, FiltMethod m);
template void filter<0u>(VoxelVolume<double>& volume, FiltMethod m);
template void filter<1u>(VoxelVolume<double>& volume, FiltMethod m);
template void filter<2u>(VoxelVolume<double>& volume, FiltMethod m);

/*!
 * \enum DiffMethod
 * Enumeration for differentiation methods that can be used.
 *
 * To incorporate a new differentiation method, add a value to this enumeration (in diff.h) and
 * provide the corresponding implementation of the method (see filter.cpp file for more information).
 *
 * In general, values on the borders will be set to zero, where no valid values are available for
 * the differentiation, i.e. within the half size of the differential operator.
 */

/*! \var DiffMethod::CentralDifference
 * This computes the central difference.
 *
 * Assuming the dimension along which the difference is computed has (valid) indices
 * \f$ n=0,...,N-1 \f$.
 *
 * The following formula applies:
 *
 *\f$
 * f'(n)=\begin{cases}
 * 0 & n=0\\
 * 0.5\cdot\left(f(n+1)-f(n-1)\right) & n=1,...,N-2\\
 * 0 & n=N-1
 * \end{cases}
 * \f$
 */

/*! \var DiffMethod::DifferenceToNext
 * This computes the difference to the next value. The border value (last index) will be set to
 * zero to preserve input dimension.
 *
 * Assuming the dimension along which the difference is computed has (valid) indices
 * \f$ n=0,...,N-1 \f$.
 *
 * The following formula applies:
 *
 *\f$
 * f'(n)=\begin{cases}
 * f(n+1)-f(n) & n=0,...,N-2\\
 * 0 & n=N-1
 * \end{cases}
 * \f$
 */

/*! \var DiffMethod::SavitzkyGolay5
 * This computes the derivative using a Savitzky-Golay-Filter of lenght 5.
 *
 * Assuming the dimension along which the difference is computed has (valid) indices
 * \f$ n=0,...,N-1 \f$.
 *
 * The following formula applies:
 *
 *\f$
 * f'(n)=\begin{cases}
 * 0 & n=0,1\\
 * 0.1\cdot\left(2f(n+2)+f(n+1)-f(n-1)-2f(n-2)\right) & n=2,...,N-3\\
 * 0 & n=N-2,N-1
 * \end{cases}
 * \f$
 */

/*! \var DiffMethod::SavitzkyGolay7
 * This computes the derivative using a Savitzky-Golay-Filter of lenght 7.
 *
 * Assuming the dimension along which the difference is computed has (valid) indices
 * \f$ n=0,...,N-1 \f$.
 *
 * The following formula applies:
 *
 *\f$
 * f'(n)=\begin{cases}
 * 0 & n=0,1,2\\
 * 1/28\cdot\left(3f(n+3)+2f(n+2)+f(n+1)-f(n-1)-2f(n-2)-3f(n-3)\right) & n=3,...,N-4\\
 * 0 & n=N-3,N-2,N-1
 * \end{cases}
 * \f$
 */

/*! \var DiffMethod::SpectralGauss3
 * This computes the derivative using a spectral derivative (Fourier-based) after a convolution with
 * a Gaussian kernel of size three: 1/4 * [1 2 1], i.e. with a standard deviation `sigma=0.7071`.
 * The filter is truncated to a size of 15, which covers 99.11% of the full filter size (in terms of
 * the sum of absolute values).
 */

/*! \var DiffMethod::SpectralGauss5
 * This computes the derivative using a spectral derivative (Fourier-based) after a convolution with
 * a Gaussian kernel of size five: 1/16 * [1 4 6 4 1], i.e. with a standard deviation `sigma=1.000`.
 * The filter is truncated to a size of 7, which covers 99.12% of the full filter size (in terms of
 * the sum of absolute values).
 */

/*! \var DiffMethod::SpectralGauss7
 * This computes the derivative using a spectral derivative (Fourier-based) after a convolution with
 * a Gaussian kernel of size seven: 1/64 * [1 6 15 20 15 6 1], i.e. with a standard deviation
 * `sigma=1.225`.
 * The filter is truncated to a size of 7, which covers 99.13% of the full filter size (in terms of
 * the sum of absolute values).
 */

/*! \var DiffMethod::SpectralGauss9
 * This computes the derivative using a spectral derivative (Fourier-based) after a convolution with
 * a Gaussian kernel of size nine: 1/256 * [1 8 28 56 70 56 28 8 1], i.e. with a standard deviation
 * `sigma=1.414`.
 * The filter is truncated to a size of 9, which covers 99.81% of the full filter size (in terms of
 * the sum of absolute values).
 */

/*!
 * \enum FiltMethod
 * Enumeration for filter methods that can be used.
 *
 * To incorporate a new filter method, add a value to this enumeration (in filter.h) and provide the
 * corresponding implementation of the method (see filter.cpp file for more information).
 *
 * In general, values on the borders will be set to zero, where no valid values are available for
 * the filtering, i.e. within the half filter size.
 */

/*! \var FiltMethod::RamLak43
 * This computes the truncated RamLak filter (discretization of the ramp filter) using only the
 * central 43 filter elements. This covers 99.08% of the full filter size in terms of the sum of
 * absolute values.
 * The elements are defined as
 *
 * \f$
 * h(n)=\begin{cases}
 * \frac{1}{4} & n=0\\
 * -\frac{1}{(\pi n)^{2}} & n\text{ odd}\\
 * 0 & n\text{ even}
 * \end{cases}
 * \f$
 *
 * where 0 is the central element.
 */

} // namespace imgproc
} // namespace CTL
