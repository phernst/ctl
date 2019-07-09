#ifndef DIFF_H
#define DIFF_H

typedef unsigned int uint; //!< Alias for unsigned int

namespace CTL {

template <typename T>
class Chunk2D;
template <typename T>
class VoxelVolume;

namespace imgproc {

enum Method { CentralDifference,
              DifferenceToNext,
              SavitzkyGolay5,
              SavitzkyGolay7 };

// partial derivatives
template <uint dim>
void diff(Chunk2D<float>& image, Method m = CentralDifference);
template <uint dim>
void diff(Chunk2D<double>& image, Method m = CentralDifference);

template <uint dim>
void diff(VoxelVolume<float>& volume, Method m = CentralDifference);
template <uint dim>
void diff(VoxelVolume<double>& volume, Method m = CentralDifference);

} // namespace imgproc
} // namespace CTL

/*! \file */
///@{

///@}

#endif // DIFF_H
