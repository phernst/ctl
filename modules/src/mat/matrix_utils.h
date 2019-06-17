#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include "matrix_types.h"
#include <QVector>
#include <QVariant>

namespace CTL {
namespace mat {

// # converter and maker functions

// conversion of the Matrix content to a QVector
template<uint Rows, uint Cols>
QVector<double> toQVector(const Matrix<Rows,Cols> &matrix);

// rotation matrix
Matrix3x3 rotationMatrix(double angle, Qt::Axis axis);

// NxN identity matrix
template<uint N>
Matrix<N,N> eye();

// cross product
Vector3x1 cross(const Vector3x1 & l, const Vector3x1 & r);

// concatenation
template<uint Rows, uint Cols1, uint Cols2>
Matrix<Rows,Cols1+Cols2>
horzcat(const Matrix<Rows,Cols1> & m1, const Matrix<Rows,Cols2> & m2);

template<uint Rows1, uint Rows2, uint Cols>
Matrix<Rows1+Rows2,Cols>
vertcat(const Matrix<Rows1,Cols> & m1, const Matrix<Rows2,Cols> & m2);

// # structs

struct PairMat3x3 {
    Matrix3x3 Q;
    Matrix3x3 R;
};

struct Location {
    Vector3x1 position = Vector3x1(0.0);
    Matrix3x3 rotation = mat::eye<3>();

    Location() = default;
    Location(const Vector3x1 &pos, const Matrix3x3 &rot);

    QVariant toVariant() const;
    void fromVariant(const QVariant& variant);
};

} // namespace mat
} // namespace CTL

#include "matrix_utils.tpp"

#endif // MATRIX_UTILS_H
