#ifndef CTL_MATRIX_ALGORITHM_H
#define CTL_MATRIX_ALGORITHM_H

#include "matrix_types.h"

namespace CTL {
namespace mat {

// ## QR/RQ decomposition
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
double det(const Matrix3x3& m);

// compute a normalized orthogonal vector
mat::Matrix<3, 1> orthonormalTo(const mat::Matrix<3, 1>& v);

} // namespace mat
} // namespace CTL

#endif // CTL_MATRIX_ALGORITHM_H
