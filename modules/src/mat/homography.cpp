#include "homography.h"
#include "matrix_utils.h"

namespace CTL {
namespace mat {

// # 2D

Homography2D::Homography2D(const Matrix<3, 3> &other)
    : Matrix<3, 3>(other)
{
}

Homography2D::Homography2D(const Matrix<2, 1>& translation)
    : Matrix<3, 3>{ 1., 0., translation.get<0>(),
                    0., 1., translation.get<1>(),
                    0., 0.,                   1. }
{
}

Homography2D::Homography2D(const Matrix<2, 2>& rotation)
    : Matrix<3, 3>{ rotation.get<0, 0>(), rotation.get<0, 1>(), 0.,
                    rotation.get<1, 0>(), rotation.get<1, 1>(), 0.,
                                      0.,                   0., 1. }
{
}

Homography2D::Homography2D(double angle)
{
    const auto sinVal = std::sin(angle);
    const auto cosVal = std::cos(angle);
    this->get<0, 0>() = cosVal; this->get<0, 1>() = -sinVal; this->get<0, 2>() = 0.;
    this->get<1, 0>() = sinVal; this->get<1, 1>() =  cosVal; this->get<1, 2>() = 0.;
    this->get<2, 0>() =     0.; this->get<2, 1>() =      0.; this->get<2, 2>() = 1.;
}

Homography2D::Homography2D(const Matrix<2, 2>& rotation, const Matrix<2, 1>& translation)
    : Matrix<3, 3>{ rotation.get<0, 0>(), rotation.get<0, 1>(), translation.get<0>(),
                    rotation.get<1, 0>(), rotation.get<1, 1>(), translation.get<1>(),
                                      0.,                   0.,                   1. }
{
}

Homography2D::Homography2D(double angle, const Matrix<2, 1>& translation)
{
    const auto sinVal = std::sin(angle);
    const auto cosVal = std::cos(angle);
    this->get<0, 0>() = cosVal; this->get<0, 1>() = -sinVal; this->get<0, 2>() = translation.get<0>();
    this->get<1, 0>() = sinVal; this->get<1, 1>() =  cosVal; this->get<1, 2>() = translation.get<1>();
    this->get<2, 0>() =     0.; this->get<2, 1>() =      0.; this->get<2, 2>() =                   1.;
}

Homography2D Homography2D::passive(const Matrix<2, 2>& rotation)
{
    return Homography2D(rotation.transposed());
}

Homography2D Homography2D::passive(double angle)
{
    return Homography2D(-angle);
}

Homography2D Homography2D::passive(const Matrix<2, 1>& translation)
{
    return Homography2D(-translation);
}

Homography2D Homography2D::passive(const Matrix<2, 2>& rotation, const Matrix<2, 1>& translation)
{
    auto Rt = rotation.transposed();
    return { Rt, -(Rt * translation) };
}

Homography2D Homography2D::passive(double angle, const Matrix<2, 1>& translation)
{
    const auto sinVal = std::sin(angle);
    const auto cosVal = std::cos(angle);
    Matrix<2, 2> Rt = { cosVal, sinVal,
                       -sinVal, cosVal };
    return { Rt, -(Rt * translation) };
}

// # 3D

Homography3D::Homography3D(const Matrix<4, 4>& other)
    : Matrix<4, 4>(other)
{
}

Homography3D::Homography3D(const Matrix<3, 1>& translation)
    : Matrix<4, 4>{ 1., 0., 0., translation.get<0>(),
                    0., 1., 0., translation.get<1>(),
                    0., 0., 1., translation.get<2>(),
                    0., 0., 0.,                   1. }
{
}

Homography3D::Homography3D(const Matrix<3, 3>& rotation)
    : Matrix<4, 4>{ rotation.get<0, 0>(), rotation.get<0, 1>(), rotation.get<0, 2>(), 0.,
                    rotation.get<1, 0>(), rotation.get<1, 1>(), rotation.get<1, 2>(), 0.,
                    rotation.get<2, 0>(), rotation.get<2, 1>(), rotation.get<2, 2>(), 0.,
                                      0.,                   0.,                   0., 1. }
{
}

Homography3D::Homography3D(const Matrix<3, 3>& rotation, const Matrix<3, 1>& translation)
    : Matrix<4, 4>{
          rotation.get<0, 0>(), rotation.get<0, 1>(), rotation.get<0, 2>(), translation.get<0>(),
          rotation.get<1, 0>(), rotation.get<1, 1>(), rotation.get<1, 2>(), translation.get<1>(),
          rotation.get<2, 0>(), rotation.get<2, 1>(), rotation.get<2, 2>(), translation.get<2>(),
                            0.,                   0.,                   0.,                   1. }
{
}

Homography3D Homography3D::passive(const Matrix<3, 3>& rotation)
{
    return Homography3D(rotation.transposed());
}

Homography3D Homography3D::passive(const Matrix<3, 1>& translation)
{
    return Homography3D(-translation);
}

Homography3D Homography3D::passive(const Matrix<3, 3>& rotation, const Matrix<3, 1>& translation)
{
    auto Rt = rotation.transposed();
    return { Rt, -(Rt * translation) };
}

} // namespace mat
} // namespace CTL
