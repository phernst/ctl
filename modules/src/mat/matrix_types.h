#ifndef MATRIX_TYPES_H
#define MATRIX_TYPES_H

#include "projectionmatrix.h"

namespace CTL {

using mat::ProjectionMatrix;

// # typedefs
typedef mat::Matrix<3, 3> Matrix3x3;
typedef mat::Matrix<3, 1> Vector3x1;

} // namespace CTL

#endif // MATRIX_TYPES_H
