#ifndef HOMOGRAPHY_H
#define HOMOGRAPHY_H

#include "matrix.h"
#include <qnamespace.h>

namespace CTL {
namespace mat {

class Homography2D : public Matrix<3, 3>
{
    // ctors for full compatibility to base class
    Homography2D() = default;
    Homography2D(const Matrix<3, 3>& other);
    using Matrix<3, 3>::Matrix;
    // assignment of a base class object
    using Matrix<3, 3>::operator=;

    // specialized ctors
    // translation
    Homography2D(const Matrix<2, 1>& translation);
    // rotation
    Homography2D(const Matrix<2, 2>& rotation);
    Homography2D(double angle);
    // Euclidian transform
    Homography2D(const Matrix<2, 1>& translation, const Matrix<2, 2>& rotation);
    Homography2D(const Matrix<2, 1>& translation, double angle);
};

class Homography3D : public Matrix<4, 4>
{
public:
    // ctors for full compatibility to base class
    Homography3D() = default;
    Homography3D(const Matrix<4, 4>& other);
    using Matrix<4, 4>::Matrix;
    // assignment of a base class object
    using Matrix<4, 4>::operator=;

    // specialized ctors
    // translation
    Homography3D(const Matrix<3, 1>& translation);
    // rotation
    Homography3D(const Matrix<3, 3>& rotation);
    Homography3D(double angle, Qt::Axis axis);
    // Euclidian transform
    Homography3D(const Matrix<3, 1>& translation, const Matrix<3, 3>& rotation);
    Homography3D(const Matrix<3, 1>& translation, double angle, Qt::Axis axis);
};

} // namespace mat
} // namespace CTL

#endif // HOMOGRAPHY_H
