#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include "matrix_types.h"
#include <QVariant>
#include <QVector>

namespace CTL {
namespace mat {

// conversion of the Matrix content to a QVector
template <uint Rows, uint Cols>
QVector<double> toQVector(const Matrix<Rows, Cols>& matrix);

// rotation matrix and related
Matrix3x3 rotationMatrix(double angle, Qt::Axis axis);
Matrix3x3 rotationMatrix(double angle, const Vector3x1& axis);
Matrix3x3 rotationMatrix(const Vector3x1& axis) noexcept;
Vector3x1 rotationAxis(const Matrix3x3& rotMat, bool lengthEqualsAngle = true);
double rotationAngle(const Matrix3x3& rotMat);

// # CTL helper struct
struct Location
{
    Vector3x1 position = Vector3x1(0.0);
    Matrix3x3 rotation = mat::eye<3>();

    Location() = default;
    Location(const Vector3x1& pos, const Matrix3x3& rot);

    QVariant toVariant() const;
    void fromVariant(const QVariant& variant);
};

} // namespace mat
} // namespace CTL

#include "matrix_utils.tpp"

#endif // MATRIX_UTILS_H
