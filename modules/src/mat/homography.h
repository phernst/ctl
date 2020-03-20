#ifndef CTL_HOMOGRAPHY_H
#define CTL_HOMOGRAPHY_H

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
    // rotation
    explicit Homography2D(const Matrix<2, 2>& rotation);
    explicit Homography2D(double angle);
    // translation
    explicit Homography2D(const Matrix<2, 1>& translation);
    // Euclidian transform
    Homography2D(const Matrix<2, 2>& rotation, const Matrix<2, 1>& translation);
    Homography2D(double angle, const Matrix<2, 1>& translation);

    // "passive" factories for describing the transformation of the coordinate system
    static Homography2D passive(const Matrix<2, 2>& rotation);
    static Homography2D passive(double angle);
    static Homography2D passive(const Matrix<2, 1>& translation);
    static Homography2D passive(const Matrix<2, 2>& rotation, const Matrix<2, 1>& translation);
    static Homography2D passive(double angle, const Matrix<2, 1>& translation);
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
    // rotation
    explicit Homography3D(const Matrix<3, 3>& rotation);
    Homography3D(double angle, Qt::Axis axis);
    // translation
    explicit Homography3D(const Matrix<3, 1>& translation);
    // Euclidian transform
    Homography3D(const Matrix<3, 3>& rotation, const Matrix<3, 1>& translation);
    Homography3D(double angle, Qt::Axis axis, const Matrix<3, 1>& translation);

    // "passive" factories for describing the transformation of the coordinate system
    static Homography3D passive(const Matrix<3, 3>& rotation);
    static Homography3D passive(double angle, Qt::Axis axis);
    static Homography3D passive(const Matrix<3, 1>& translation);
    static Homography3D passive(const Matrix<3, 3>& rotation, const Matrix<3, 1>& translation);
    static Homography3D passive(double angle, Qt::Axis axis, const Matrix<3, 1>& translation);
};

} // namespace mat
} // namespace CTL

#endif // CTL_HOMOGRAPHY_H
