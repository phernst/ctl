#ifndef MATRIX_ALGORITHM_H
#define MATRIX_ALGORITHM_H

#include "matrix_types.h"

namespace CTL {
namespace mat {

// ## QR decomposition
struct PairMat3x3
{
    Matrix3x3 Q;
    Matrix3x3 R;
};
PairMat3x3 QRdecomposition(const Matrix3x3 &A);
PairMat3x3 RQdecomposition(const Matrix3x3 &A, bool unique = true, bool normalize = true);

// cross product
Vector3x1 cross(const Vector3x1& l, const Vector3x1& r);

// determinant
double det(const Matrix3x3 & A);

} // namespace mat
} // namespace CTL

#endif // MATRIX_ALGORITHM_H
