#ifndef CTL_FILTER_H
#define CTL_FILTER_H

typedef unsigned int uint; //!< Alias for unsigned int

namespace CTL {

template <typename T>
class Chunk2D;
template <typename T>
class VoxelVolume;

namespace imgproc {

enum FiltMethod {
    Gauss3,     // filter size: 3
    Gauss5,     // filter size: 5
    Gauss7,     // filter size: 7
    Average3,   // filter size: 3
    Median3,    // filter size: 3
    MedianAbs3, // filter size: 3
    MaxAbs3,    // filter size: 3
    RamLak,     // filter size: 1407
};

// one-dimensional filter
template <uint dim>
void filter(Chunk2D<float>& image, FiltMethod m);
template <uint dim>
void filter(Chunk2D<double>& image, FiltMethod m);

template <uint dim>
void filter(VoxelVolume<float>& volume, FiltMethod m);
template <uint dim>
void filter(VoxelVolume<double>& volume, FiltMethod m);

// available specializations (explicit instantiation declaration)
extern template void filter<0u>(VoxelVolume<float>& volume, FiltMethod m);
extern template void filter<1u>(VoxelVolume<float>& volume, FiltMethod m);
extern template void filter<2u>(VoxelVolume<float>& volume, FiltMethod m);
extern template void filter<0u>(VoxelVolume<double>& volume, FiltMethod m);
extern template void filter<1u>(VoxelVolume<double>& volume, FiltMethod m);
extern template void filter<2u>(VoxelVolume<double>& volume, FiltMethod m);

extern template void filter<0u>(Chunk2D<float>& image, FiltMethod m);
extern template void filter<1u>(Chunk2D<float>& image, FiltMethod m);
extern template void filter<0u>(Chunk2D<double>& image, FiltMethod m);
extern template void filter<1u>(Chunk2D<double>& image, FiltMethod m);

} // namespace imgproc
} // namespace CTL

/*! \file */
///@{

///@}

#endif // CTL_FILTER_H
