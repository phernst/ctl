#ifndef CTL_DIFF_H
#define CTL_DIFF_H

typedef unsigned int uint; //!< Alias for unsigned int

namespace CTL {

template <typename T>
class Chunk2D;
template <typename T>
class VoxelVolume;

namespace imgproc {

enum DiffMethod { // diff enum has to have negative values (non-negative are reserved for filter)
    CentralDifference = -1, // filter size: 3
    DifferenceToNext  = -2, // filter size: 2
    SavitzkyGolay5    = -3, // filter size: 5
    SavitzkyGolay7    = -4, // filter size: 7
    SpectralGauss3    = -5, // filter size: 15
    SpectralGauss5    = -6, // filter size: 7
    SpectralGauss7    = -7, // filter size: 7
    SpectralGauss9    = -8, // filter size: 9
    SpectralCosine    = -9, // filter size: 11
};

// partial derivatives
template <uint dim>
void diff(Chunk2D<float>& image, DiffMethod m = CentralDifference);
template <uint dim>
void diff(Chunk2D<double>& image, DiffMethod m = CentralDifference);

template <uint dim>
void diff(VoxelVolume<float>& volume, DiffMethod m = CentralDifference);
template <uint dim>
void diff(VoxelVolume<double>& volume, DiffMethod m = CentralDifference);

// available specializations (explicit instantiation declaration)
extern template void diff<0u>(Chunk2D<float>& image, DiffMethod m);
extern template void diff<1u>(Chunk2D<float>& image, DiffMethod m);
extern template void diff<0u>(Chunk2D<double>& image, DiffMethod m);
extern template void diff<1u>(Chunk2D<double>& image, DiffMethod m);

extern template void diff<0u>(VoxelVolume<float>& volume, DiffMethod m);
extern template void diff<1u>(VoxelVolume<float>& volume, DiffMethod m);
extern template void diff<2u>(VoxelVolume<float>& volume, DiffMethod m);
extern template void diff<0u>(VoxelVolume<double>& volume, DiffMethod m);
extern template void diff<1u>(VoxelVolume<double>& volume, DiffMethod m);
extern template void diff<2u>(VoxelVolume<double>& volume, DiffMethod m);

} // namespace imgproc
} // namespace CTL

/*! \file */
///@{

///@}

#endif // CTL_DIFF_H
