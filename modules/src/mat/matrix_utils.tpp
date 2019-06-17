#include "matrix_utils.h"

namespace CTL {
namespace mat {

// # converters and factories
template<uint Rows, uint Cols>
QVector<double> toQVector(const Matrix<Rows,Cols> &matrix)
{
    QVector<double> ret(Rows*Cols);
    std::copy(matrix.constBegin(),matrix.constEnd(),ret.begin());
    return ret;
}

inline Matrix3x3 rotationMatrix(double angle, Qt::Axis axis)
{
    const auto sinVal = sin(angle);
    const auto cosVal = cos(angle);
    switch(axis)
    {
    case Qt::XAxis: return {    1.0,     0.0,     0.0,
                                0.0,  cosVal, -sinVal,
                                0.0,  sinVal,  cosVal };

    case Qt::YAxis: return { cosVal,     0.0,  sinVal,
                                0.0,     1.0,     0.0,
                            -sinVal,     0.0,  cosVal };

    case Qt::ZAxis: return { cosVal, -sinVal,     0.0,
                             sinVal,  cosVal,     0.0,
                                0.0,     0.0,     1.0 };
    }
    return mat::eye<3>();
}

// identity matrix
template<uint N>
Matrix<N,N> eye() {
    static Matrix<N,N> ret(0.0);
    static bool toBeInit = true;
    if(toBeInit)
        for(uint i = 0; i<N; ++i)
            ret(i,i) = 1.0;
    toBeInit = false;
    return ret;
}

inline Vector3x1 cross(const Vector3x1 & l, const Vector3x1 & r)
{
    return { l.get<1>() * r.get<2>() - l.get<2>() * r.get<1>(),
             l.get<2>() * r.get<0>() - l.get<0>() * r.get<2>(),
             l.get<0>() * r.get<1>() - l.get<1>() * r.get<0>() };
}

// concatenation
template<uint Rows, uint Cols1, uint Cols2>
Matrix<Rows,Cols1+Cols2>
horzcat(const Matrix<Rows,Cols1> & m1, const Matrix<Rows,Cols2> & m2) {
    Matrix<Rows,Cols1+Cols2> ret;
    auto dstPtr = ret.begin();
    for(uint row = 0; row < Rows; ++row)
    {
        std::copy_n(m1[row],Cols1,dstPtr);
        dstPtr += Cols1;
        std::copy_n(m2[row],Cols2,dstPtr);
        dstPtr += Cols2;
    }
    return ret;
}
template<uint Rows1, uint Rows2, uint Cols>
Matrix<Rows1+Rows2,Cols>
vertcat(const Matrix<Rows1,Cols> & m1, const Matrix<Rows2,Cols> & m2) {
    Matrix<Rows1+Rows2,Cols> ret;
    std::copy( m1.begin(), m1.end(), ret[0] );
    std::copy( m2.begin(), m2.end(), ret[Rows1] );
    return ret;
}
// variadic versions (auto return type = C++14 feature)
#if __cplusplus >= 201402L
template<uint Rows, uint Cols1, uint Cols2, class... Matrices>
auto horzcat(const Matrix<Rows,Cols1> & m1, const Matrix<Rows,Cols2> & m2,
             const Matrices &... mats) {
    return horzcat(horzcat(m1,m2), mats...);
}
template<uint Rows1, uint Rows2, uint Cols, class... Matrices>
auto vertcat(const Matrix<Rows1,Cols> & m1, const Matrix<Rows2,Cols> & m2,
             const Matrices &... mats) {
    return vertcat(vertcat(m1,m2), mats...);
}
#endif

inline Location::Location(const Vector3x1 &pos, const Matrix3x3 &rot)
    : position(pos),
      rotation(rot)
{
}

inline QVariant Location::toVariant() const
{
    QVariantMap map;

    QVariantList pos;
    pos.reserve(3);
    pos.append(position.get<0>());
    pos.append(position.get<1>());
    pos.append(position.get<2>());

    QVariantList rot;
    rot.reserve(9);
    rot.append(rotation.get<0>());
    rot.append(rotation.get<1>());
    rot.append(rotation.get<2>());
    rot.append(rotation.get<3>());
    rot.append(rotation.get<4>());
    rot.append(rotation.get<5>());
    rot.append(rotation.get<6>());
    rot.append(rotation.get<7>());
    rot.append(rotation.get<8>());

    map.insert("position", pos);
    map.insert("rotation", rot);

    return map;
}

inline void Location::fromVariant(const QVariant &variant)
{
    auto map = variant.toMap();

    auto pos = map.value("position").toList();
    auto rot = map.value("rotation").toList();

    QVector<double> posDbl, rotDbl;

    for(const auto& val : pos)
        posDbl.append(val.toDouble());
    for(const auto& val : rot)
        rotDbl.append(val.toDouble());

    position = Vector3x1::fromContainer(posDbl, 0);
    rotation = Matrix3x3::fromContainer(rotDbl, 0);
}

} // namespace mat
} // namespace CTL

