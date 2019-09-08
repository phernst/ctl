#ifndef DIFF_H
#define DIFF_H

typedef unsigned int uint; //!< Alias for unsigned int

namespace CTL {

template <typename T>
class Chunk2D;
template <typename T>
class VoxelVolume;

namespace imgproc {

enum DiffMethod { // diff enum has to have negative values (non-negative are reserved for filter)
                  CentralDifference = -1,
                  DifferenceToNext = -2,
                  SavitzkyGolay5 = -3,
                  SavitzkyGolay7 = -4 };

// partial derivatives
template <uint dim>
void diff(Chunk2D<float>& image, DiffMethod m = CentralDifference);
template <uint dim>
void diff(Chunk2D<double>& image, DiffMethod m = CentralDifference);

template <uint dim>
void diff(VoxelVolume<float>& volume, DiffMethod m = CentralDifference);
template <uint dim>
void diff(VoxelVolume<double>& volume, DiffMethod m = CentralDifference);

} // namespace imgproc
} // namespace CTL

/*! \file */
///@{

///@}

#endif // DIFF_H
