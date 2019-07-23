#ifndef PROJECTIONMATRIX_H
#define PROJECTIONMATRIX_H

#include "matrix.h"

namespace CTL {
namespace mat {
/*!
 * \class ProjectionMatrix
 *
 * \brief Specialized sub-class of Matrix<3, 4> to represent a projection matrix.
 *
 * A ProjectionMatrix is used to discribe a projective mapping from 3d to a 2d image. The projection
 * model is a finite (pinhole) camera, which is capable to represent an ideal cone beam geometry.
 * Here, the ProjectionMatrix encodes the whole information that is required for the projective
 * mapping of a view with one X-ray point source and a flat panel detector. However, it is also
 * applicable for a curved or spherical detector, if it is composed of serveral flat panel modules.
 *
 * Note that a ProjectionMatrix can be multiplied with a scalar value without changing the geometry
 * of the scene. Due to this degree of freedom, it is preferred to standardize the scaling of the
 * projection matrices using the mehtods normalize() or normalized().
 *
 * The projective mapping works with
 * [homogeneous coordinates](https://en.wikipedia.org/wiki/Homogeneous_coordinates).
 * A 3d point in homogeneous world coordinates is multiplied by a ProjectionMatrix
 * \f$P\in\mathbb{R}^{3\times4}\f$:
 *
 * \f$\left[\begin{array}{c}
 * \tilde{x}\\
 * \tilde{y}\\
 * w
 * \end{array}\right]=P\left[\begin{array}{c}
 * X\\
 * Y\\
 * Z\\
 * 1
 * \end{array}\right]\f$
 *
 * Converting this vector back to cartesian coordinates, one obtains the 2d image/detector pixel
 * coordinates
 * \f$\left[\begin{array}{c}
 * x\\
 * y
 * \end{array}\right]=\left[\begin{array}{c}
 * \frac{\tilde{x}}{w}\\
 * \frac{\tilde{y}}{w}
 * \end{array}\right]\f$.
 *
 * Usually, the coordinate \f$\left[\begin{array}{c}
 * x=0\\
 * y=0
 * \end{array}\right]\f$ denotes the center of the upper left pixel of the image
 * (x-axis points from left to right and the y-axis from top to bottom).
 *
 * The elements of the ProjectionMatrix are commonly subdivided into two blocks
 * \f$M\in\mathbb{R}^{3\times3}\f$ and
 * \f$\mathbf{p}_{4}\in\mathbb{R}^{3\times1}\f$:
 *
 * \f$P=\left[\begin{array}{cccc}
 * P_{11} & P_{12} & P_{13} & P_{14}\\
 * P_{21} & P_{22} & P_{23} & P_{24}\\
 * P_{31} & P_{32} & P_{33} & P_{34}
 * \end{array}\right]=\left[\begin{array}{cc}
 * M & \mathbf{p}_{4}\end{array}\right]\f$.
 *
 *
 * Furthermore, a ProjectionMatrix can be composed/decomposed from/in extrinsic and intrinsic
 * parameters:
 * \li extrinsic parameters: rotation matrix \f$R\in\mathbb{R}^{3\times3}\f$ and
 * source position \f$\mathbf{c}\in\mathbb{R}^{3\times1}\f$
 * (or translation vector \f$\mathbf{t}\in\mathbb{R}^{3\times1}\f$)
 * \li intrinsic parameters: calibration matrix \f$K\in\mathbb{R}^{3\times3}\f$ (upper triangular
 * matrix)
 *
 * Here, the (de)composition can be stated as \f$P=K\,\left[\begin{array}{cc}
 * R & \mathbf{t}\end{array}\right]\f$. The relation between translation vector and source position
 * is given by \f$\mathbf{t}=-R\mathbf{c}\f$.
 * The intrisic matrix \f$K\f$ is composed as follows:
 *
 * \f$K=c\left[\begin{array}{ccc}
 * f_{x} & s & p_{x}\\
 * 0 & f_{y} & p_{y}\\
 * 0 & 0 & 1
 * \end{array}\right]\f$.
 *
 * \li \f$c\f$: an abitrary scale
 * \li \f$f_x\f$/\f$f_y\f$: the focal length (source to detector distance) in numbers of pixels with the x/y spacing
 * \li \f$p_x\f$/\f$p_y\f$: x/y pixel coordinate of the principal point
 * \li \f$s\f$: skew coefficient, which is zero if the detector coordinate system is orthogonal
 *
 * The principal point is the intersection of the detector plane and the principal ray, which is the
 * ray that hits perpendicularly the detector.
 *
 * \sa M(), p4(), compose()
 */
class ProjectionMatrix : public Matrix<3, 4>
{
public:
    // normalization modes for directionSourceToPixel()
    enum NormalizationMode {
        NoNormalization = 0,
        NormalizeAsUnitVector = 1,
        NormalizeByX = 2,
        NormalizeByY = 3,
        NormalizeByChannel = 2,
        NormalizeByRow = 3
    };

    ProjectionMatrix() = default;
    ProjectionMatrix(const Matrix<3, 4>& other);
    using Matrix<3, 4>::Matrix;
    using Matrix<3, 4>::operator=;

    // # factories
    static ProjectionMatrix compose(const Matrix<3, 3>& M, const Matrix<3, 1>& p4);
    static ProjectionMatrix
    compose(const Matrix<3, 3>& K, const Matrix<3, 3>& R, const Matrix<3, 1>& source);

    // # modifications
    void shiftDetectorOrigin(const Matrix<2, 1>& translation);
    void shiftDetectorOrigin(double translationX, double translationY);
    void changeDetectorResolution(double resamplingFactor);
    void changeDetectorResolution(double resamplingFactorX, double resamplingFactorY);
    void normalize();

    // # getter
    // normalized by normal vector of principal plane [P(2,0) P(2,1) P(2,2)]
    ProjectionMatrix normalized() const;
    // common submatrices: P = [M|p4]
    Matrix<3, 3> M() const;
    Matrix<3, 1> p4() const;
    // direction vectors
    Matrix<3, 1> directionSourceToPixel(const Matrix<2, 1>& pixelCoordinates,
                                        NormalizationMode normalizationMode = NoNormalization) const;
    Matrix<3, 1> directionSourceToPixel(double x, double y,
                                        NormalizationMode normalizationMode = NoNormalization) const;
    Matrix<3, 1> principalRayDirection() const;
    // extrinsic parameters
    Matrix<3, 1> translationCTS() const;
    Matrix<3, 1> sourcePosition() const;
    Matrix<3, 3> rotationMatR() const;
    // intrinsic parameters
    Matrix<3, 3> intrinsicMatK() const;
    // intrinsic convenience functions
    Matrix<2, 1> principalPoint() const;
    Matrix<2, 1> focalLength() const;
    double skewCoefficient() const;
    double magnificationX(double X, double Y, double Z) const;
    double magnificationX(const Matrix<3, 1>& worldCoordinate = { 0.0, 0.0, 0.0 }) const;
    double magnificationY(double X, double Y, double Z) const;
    double magnificationY(const Matrix<3, 1>& worldCoordinate = { 0.0, 0.0, 0.0 }) const;

    // # other/convenience
    Matrix<2, 1> projectOntoDetector(double X, double Y, double Z) const;
    Matrix<2, 1> projectOntoDetector(const Matrix<3, 1>& worldCoordinate) const;
};


/*!
 * Constructs a projection matrix from an \a other Matrix<3,4> (base class object).
 */
inline ProjectionMatrix::ProjectionMatrix(const Matrix<3, 4>& other)
    : Matrix<3, 4>(other)
{
}

} // namespace mat
} // namespace CTL

/*! \file */

#endif // PROJECTIONMATRIX_H
