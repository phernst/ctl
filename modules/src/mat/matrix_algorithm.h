#ifndef MATRIX_ALGORITHM_H
#define MATRIX_ALGORITHM_H

#include "matrix_utils.h"

namespace CTL {
namespace mat {

// ## algorithms
PairMat3x3 QRdecomposition(const Matrix3x3 &A);
PairMat3x3 RQdecomposition(const Matrix3x3 &A, bool unique = true, bool normalize = true);

double det(const Matrix3x3 & A);

} // namespace mat
} // namespace CTL

#endif // MATRIX_ALGORITHM_H
