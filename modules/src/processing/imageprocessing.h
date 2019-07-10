#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

/*
 * Wrapping header that provides all symbols from the `CTL::imgproc` namespace.
 */

#include "diff.h"

namespace CTL {

namespace mat {
template <uint Rows, uint Cols>
class Matrix;
}

namespace imgproc
{
    void cosWeighting(Chunk2D<float>& proj, const mat::Matrix<3,3>& K);

} // namespace imgproc
} // namespace CTL

#endif // IMAGEPROCESSING_H
