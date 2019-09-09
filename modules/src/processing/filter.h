#ifndef FILTER_H
#define FILTER_H

typedef unsigned int uint; //!< Alias for unsigned int

namespace CTL {

template <typename T>
class Chunk2D;
template <typename T>
class VoxelVolume;

namespace imgproc {

enum FiltMethod { Gauss3,
                  Average3,
                  Median3,
                  MedianAbs3,
                  MaxAbs3 };

// one-dimensional filter
template <uint dim>
void filter(Chunk2D<float>& image, FiltMethod m);
template <uint dim>
void filter(Chunk2D<double>& image, FiltMethod m);

template <uint dim>
void filter(VoxelVolume<float>& volume, FiltMethod m);
template <uint dim>
void filter(VoxelVolume<double>& volume, FiltMethod m);

} // namespace imgproc
} // namespace CTL

/*! \file */
///@{

///@}

#endif // FILTER_H
