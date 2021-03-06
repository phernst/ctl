#include "matrix_utils.h"

namespace CTL {
namespace mat {

// # converters and factories
template <uint Rows, uint Cols>
QVector<double> toQVector(const Matrix<Rows, Cols>& matrix)
{
    QVector<double> ret(Rows * Cols);
    std::copy(matrix.constBegin(), matrix.constEnd(), ret.begin());
    return ret;
}

/*!
 * Return a 3D rotation matrix that is constructed from a rotation \a axis and an \a angle in rad.
 * The rotation \a axis can be one of the three coordiante axis `Qt::XAxis`, `Qt::YAxis` or
 * `Qt::ZAxis`.
 * Throws if the specified \a axis is invalid, i.e. it is not part of the `Qt::Axis` enumeration.
 */
inline Matrix3x3 rotationMatrix(double angle, Qt::Axis axis)
{
    const auto sinVal = std::sin(angle);
    const auto cosVal = std::cos(angle);
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
    throw std::runtime_error("rotationMatrix(double angle, Qt::Axis axis): "
                             "No valid axis specified.");
}

/*!
 * Return a 3D rotation matrix that is constructed from a rotation \a axis and an \a angle in rad.
 * Throws if the norm of the axis is close to zero.
 */
inline Matrix3x3 rotationMatrix(double angle, const Vector3x1& axis)
{
    constexpr auto tol1 = 1.0e-7;
    constexpr auto tol2 = 1.0e-12;

    const auto axisLength = axis.norm();

    if(std::abs(angle) < tol1)
        return mat::eye<3>();
    if(axisLength < tol2)
        throw std::runtime_error("rotationMatrix(double angle, const Vector3x1& axis): "
                                 "Rotation axis is singular.");

    // convert axis to unit vector
    const double x = axis.get<0>() / axisLength;
    const double y = axis.get<1>() / axisLength;
    const double z = axis.get<2>() / axisLength;

    const double si = std::sin(angle);
    const double co = std::cos(angle);
    const double c_ = 1.0 - co;
    const double tmp[] = { x * y * c_, z * si, x * z * c_, y * z * c_, y * si, x * si };

    return { x * x * c_ + co, tmp[0] - tmp[1], tmp[2] + tmp[4],
             tmp[0] + tmp[1], y * y * c_ + co, tmp[3] - tmp[5],
             tmp[2] - tmp[4], tmp[3] + tmp[5], z * z * c_ + co };
}

/*!
 * Same as return `rotationMatrix(axis.norm(), axis)`, but does not throw.
 */
inline Matrix3x3 rotationMatrix(const Vector3x1& axis) noexcept
{
    constexpr auto tol = 1.0e-7;

    const auto angle = axis.norm();

    if(angle < tol)
        return mat::eye<3>();

    // convert axis to unit vector
    const double x = axis.get<0>() / angle;
    const double y = axis.get<1>() / angle;
    const double z = axis.get<2>() / angle;

    const double si = std::sin(angle);
    const double co = std::cos(angle);
    const double c_ = 1.0 - co;
    const double tmp[] = { x * y * c_, z * si, x * z * c_, y * z * c_, y * si, x * si };

    return { x * x * c_ + co, tmp[0] - tmp[1], tmp[2] + tmp[4],
             tmp[0] + tmp[1], y * y * c_ + co, tmp[3] - tmp[5],
             tmp[2] - tmp[4], tmp[3] + tmp[5], z * z * c_ + co };
}

inline Vector3x1 rotationAxis(const Matrix3x3& rotMat, bool lengthEqualsAngle)
{
    Vector3x1 ret{ rotMat(2,1) - rotMat(1,2),
                   rotMat(0,2) - rotMat(2,0),
                   rotMat(1,0) - rotMat(0,1) };

    if(lengthEqualsAngle)
    {
        const auto norm = ret.norm();
        const auto angle = std::asin(0.5 * norm);
        ret *= angle / norm;
    }

    return ret;
}

inline double rotationAngle(const Matrix3x3& rotMat)
{
    const auto trace = rotMat.get<0, 0>() + rotMat.get<1, 1>() + rotMat.get<2, 2>();
    const auto arg = 0.5 * (trace - 1);
    return std::acos(std::min(arg, 1.0)); // limit by 1 for numerical safety
}

inline Location::Location(const Vector3x1& pos, const Matrix3x3& rot)
    : position(pos)
    , rotation(rot)
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

inline void Location::fromVariant(const QVariant& variant)
{
    auto map = variant.toMap();

    auto pos = map.value("position").toList();
    auto rot = map.value("rotation").toList();

    QVector<double> posDbl, rotDbl;

    for(const auto& val : qAsConst(pos))
        posDbl.append(val.toDouble());
    for(const auto& val : qAsConst(rot))
        rotDbl.append(val.toDouble());

    position = Vector3x1::fromContainer(posDbl, 0);
    rotation = Matrix3x3::fromContainer(rotDbl, 0);
}

} // namespace mat
} // namespace CTL
