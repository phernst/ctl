#include "projectionmatrix.h"
#include "matrix_algorithm.h"

namespace CTL {
namespace mat {
/*!
 * \enum ProjectionMatrix::NormalizationMode
 * This enumeration is a switch for the method directionSourceToPixel() that sets the procedure for the
 * normalization of the returned vector.
 *
 * Type                                    | Type-ID
 * --------------------------------------- | -------------
 * ProjectionMatrix::NoNormalization       | 0
 * ProjectionMatrix::NormalizeAsUnitVector | 1
 * ProjectionMatrix::NormalizeByX          | 2
 * ProjectionMatrix::NormalizeByY          | 3
 * ProjectionMatrix::NormalizeByChannel    | 2
 * ProjectionMatrix::NormalizeByRow        | 3
 */

/*!
 * \fn ProjectionMatrix::ProjectionMatrix()
 *
 * Default constructor. Constructs a projection matrix without any initialization.
 */

/*!
 * \fn ProjectionMatrix::ProjectionMatrix(double fillValue)
 *
 * Constructs a projection matrix by initializing each matrix entry with \a fillValue.
 */

/*!
 * \fn ProjectionMatrix::ProjectionMatrix(const Matrix<3, 4>& other)
 *
 * Constructs a projection matrix from an \a other Matrix<3,4> (base class object).
 */

/*!
 * Composes a ProjectionMatrix from sub-blocks \a M and \a p4: \f$P = \left[\begin{array}{cc}
 * M & \mathbf{p}_{4}\end{array}\right]\f$
 */
ProjectionMatrix ProjectionMatrix::compose(const Matrix<3, 3>& M, const Matrix<3, 1>& p4)
{
    return mat::horzcat(M, p4);
}

/*!
 * Composes a ProjectionMatrix from intrinsic calibration matrix \a K, extrinsic rotation
 * matrix \a R und \a source position \f$\mathbf{c}\f$: \f$P = KR\,\left[\begin{array}{cc}
 * I & -\mathbf{c}\end{array}\right]\f$, where \f$I\f$ is a 3x3 identity matrix.
 *
 * \sa intrinsicMatK(), rotationMatR(), sourcePosition()
 */
ProjectionMatrix
ProjectionMatrix::compose(const Matrix<3, 3>& K, const Matrix<3, 3>& R, const Matrix<3, 1>& source)
{
    auto M = K * R;
    return compose(M, -M * source);
}

/*!
 * Shift image (detector) coordiante origin (upper left image corner), e.g. when cropping the image
 * in a way that the upper left corner is affected.
 * The \a translation = \f$[tx,ty]\f$ of the image frame is performed by a matrix multiplication
 * from the left-hand side:
 *
 * \f$P\rightarrow\left(\begin{array}{ccc}
 *  1 & 0 & -tx\\
 *  0 & 1 & -ty\\
 *  0 & 0 & 1
 * \end{array}\right)P\f$
 *
 * \sa shiftDetectorOrigin(double translationX, double translationY)
 */
void ProjectionMatrix::shiftDetectorOrigin(const Matrix<2, 1>& translation)
{
    shiftDetectorOrigin(translation.get<0>(), translation.get<1>());
}

/*!
 * Convenience function that does the same as
 * shiftDetectorOrigin(const Matrix<2, 1>& translation).
 */
void ProjectionMatrix::shiftDetectorOrigin(double translationX, double translationY)
{
    get<0, 0>() -= translationX * get<2, 0>();
    get<0, 1>() -= translationX * get<2, 1>();
    get<0, 2>() -= translationX * get<2, 2>();
    get<0, 3>() -= translationX * get<2, 3>();

    get<1, 0>() -= translationY * get<2, 0>();
    get<1, 1>() -= translationY * get<2, 1>();
    get<1, 2>() -= translationY * get<2, 2>();
    get<1, 3>() -= translationY * get<2, 3>();
}

/*!
 * Increases the number of detector pixels by the same \a resamplingFactor for both dimensions.
 *
 * \sa changeDetectorResolution(double resamplingFactorX, double resamplingFactorY)
 */
void ProjectionMatrix::changeDetectorResolution(double resamplingFactor)
{
    changeDetectorResolution(resamplingFactor, resamplingFactor);
}

/*!
 * Increases the number of detector pixels by a \a resamplingFactorX and \a resamplingFactorY
 * for each dimensions respectively.
 *
 * \sa changeDetectorResolution(double resamplingFactor)
 */
void ProjectionMatrix::changeDetectorResolution(double resamplingFactorX, double resamplingFactorY)
{
    get<0, 0>() *= resamplingFactorX;
    get<0, 1>() *= resamplingFactorX;
    get<0, 2>() *= resamplingFactorX;
    get<0, 3>() *= resamplingFactorX;

    get<1, 0>() *= resamplingFactorY;
    get<1, 1>() *= resamplingFactorY;
    get<1, 2>() *= resamplingFactorY;
    get<1, 3>() *= resamplingFactorY;
}

/*!
 * Normalizes the current projection matrix.
 * \sa normalized()
 */
void ProjectionMatrix::normalize()
{
    double denom = Matrix<3, 1>({ get<2, 0>(), get<2, 1>(), get<2, 2>() }).norm();
    denom = std::copysign(denom, mat::det(M()));
    Q_ASSERT(!qFuzzyIsNull(denom));
    *this /= denom;
}

/*!
 * Returns a normalized ProjectionMatrix by dividing by the norm of the principal ray direction
 * \f$[P_{31}, P_{32}, P_{33}]\f$ (also referred to as the normal vector of the principal plane).
 * Moreover, it normalizes the sign, so that the determinant of
 * \f$M\f$ is positive \f$(P = \left[\begin{array}{cc}
 * M & \mathbf{p}_{4}\end{array}\right])\f$.
 */
ProjectionMatrix ProjectionMatrix::normalized() const
{
    double denom = Matrix<3, 1>({ get<2, 0>(), get<2, 1>(), get<2, 2>() }).norm();
    denom = std::copysign(denom, mat::det(M()));
    Q_ASSERT(!qFuzzyIsNull(denom));
    return *this / denom;
}

/*!
 * Returns submatrix \f$M\f$, whereby \f$P = \left[\begin{array}{cc}
 * M & \mathbf{p}_{4}\end{array}\right]\f$.
 */
Matrix<3, 3> ProjectionMatrix::M() const
{
    return mat::horzcat(mat::horzcat(column<0>(), column<1>()), column<2>());
}

/*!
 * Returns vector \f$\mathbf{p}_{4}\f$, whereby \f$P = \left[\begin{array}{cc}
 * M & \mathbf{p}_{4}\end{array}\right]\f$.
 */
Matrix<3, 1> ProjectionMatrix::p4() const { return column<3>(); }

// direction vectors

/*!
 * Convenience function that does the same as
 * directionSourceToPixel(double x, double y, NormalizationMode normalizationMode) const.
 */
Matrix<3, 1> ProjectionMatrix::directionSourceToPixel(const Matrix<2, 1>& pixelCoordinates,
                                                      NormalizationMode normalizationMode) const
{
    return directionSourceToPixel(pixelCoordinates.get<0>(),
                                  pixelCoordinates.get<1>(),
                                  normalizationMode);
}

/*!
 * Returns a direction vector of a ray from source to detector pixel (\a x, \a y) by calculating
 * \f$\textrm{sign}\left(\det M\right)M^{-1}\left[x,y,1\right]^{T}\f$ using RQ decomposition of
 * \f$M\f$: \f$M^{-1}=Q^{T}R^{-1}\f$.
 * The \a normalizationMode specifies the length of the returned vector:
 * \li `NoNormalization` (default): No normalization is performed. The length depend on the scaling of the ProjectionMatrix.
 * \li `NormalizeAsUnitVector`: The length equals to 1
 * \li `NormalizeByX`: The returned vector points from the source to the detector pixel and is units
 * of the pixels in x-/channel-direction. If you multiply with the physical pixel spacing in
 * x-direction, you will obtain a vector with the actual physical length.
 * \li `NormalizeByY`: Same as NormalizeByX, but w.r.t. the y-/row-direction. However, NormalizeByX is
 * recommended, since it is considered to be faster.
 * \li `NormalizeByChannel`: Identical to NormalizeByX.
 * \li `NormalizeByRow`: Identical to NormalizeByY.
 */
Matrix<3, 1> ProjectionMatrix::directionSourceToPixel(double x, double y,
                                                      NormalizationMode normalizationMode) const
{
    auto RQ = mat::RQdecomposition(M(),false,false);
    auto& R = RQ.R;
    auto& Q = RQ.Q;

    if(std::signbit(R.get<0,0>() * R.get<1,1>() * R.get<2,2>()))
        R = -R;

    // back substitution to find 'd' in R*d = [x,y,1]^t
    double dz = 1.0 / R.get<2,2>();
    double dy = (y - dz*R.get<1,2>()) / R.get<1,1>();
    double dx = (x - dy*R.get<0,1>() - dz*R.get<0,2>()) / R.get<0,0>();

    Matrix<3, 1> ret({ dx, dy, dz });
    ret = Q.transposed() * ret;

    switch (normalizationMode) {
    case NoNormalization:
        break;
    case NormalizeAsUnitVector:
        ret /= ret.norm();
        break;
    case NormalizeByX: // same as NormalizeByChannel
        ret *= fabs(R.get<0,0>());
        break;
    case NormalizeByY: // same as NormalizeByRow
        double aa = R.get<0,0>() * R.get<0,0>(); // a b c
        double bb = R.get<0,1>() * R.get<0,1>(); // 0 d e
        double dd = R.get<1,1>() * R.get<1,1>(); // 0 0 f
        double scale = sqrt(aa * dd / (aa + bb));
        ret *= scale;
        break;
    }
    return ret;
}

/*!
 * Returns unit direction vector of the ray that hits perpendicularly the detector.
 */
Matrix<3, 1> ProjectionMatrix::principalRayDirection() const
{
    Matrix<3, 1> ret({ get<2, 0>(), get<2, 1>(), get<2, 2>() });
    const auto vecNorm = ret.norm();
    Q_ASSERT(!qFuzzyIsNull(vecNorm));
    const double scale = std::copysign(1.0 / vecNorm, mat::det(M()));
    ret *= scale;
    return ret;
}

// extrinsic parameters

/*!
 * Returns the source position (extrinsic parameters).
 *
 * \sa compose(const Matrix<3, 3>& K, const Matrix<3, 3>& R, const Matrix<3, 1>& source)
 */
Matrix<3, 1> ProjectionMatrix::sourcePosition() const
{
    using mat::det;
    using mat::horzcat;
    // normalization to convert homogeneous to cartesian coordinates
    double hom2cart = -det(horzcat(horzcat(column<0>(), column<1>()), column<2>()));
    Q_ASSERT(!qFuzzyIsNull(hom2cart));
    Matrix<3, 1> ret({ det(horzcat(horzcat(column<1>(), column<2>()), column<3>())),
                      -det(horzcat(horzcat(column<0>(), column<2>()), column<3>())),
                       det(horzcat(horzcat(column<0>(), column<1>()), column<3>())) });
    ret /= hom2cart;
    return ret;
}

/*!
 * Returns the rotation matrix (extrinsic parameters).
 *
 * \sa compose(const Matrix<3, 3>& K, const Matrix<3, 3>& R, const Matrix<3, 1>& source)
 */
Matrix<3, 3> ProjectionMatrix::rotationMatR() const
{
    return mat::RQdecomposition(M(), true, false).Q;
}

// intrinsic parameters

/*!
 * Returns the calibration matrix (intrinsic parameters).
 *
 * \sa compose(const Matrix<3, 3>& K, const Matrix<3, 3>& R, const Matrix<3, 1>& source)
 */
Matrix<3, 3> ProjectionMatrix::intrinsicMatK() const
{
    return mat::RQdecomposition(M(), true, true).R;
}

// intrinsic convenience functions

/*!
 * Returns the principal point (intrinsic parameters), i.e. the pixel coordinates corresponding to
 * the principal ray (ray that hits perpendicularly the detector).
 */
Matrix<2, 1> ProjectionMatrix::principalPoint() const
{
    auto M_ = M();
    auto pP = M_ * M_.row<2>().transposed();
    Matrix<2, 1> ret({ pP.get<0>(), pP.get<1>() });
    Q_ASSERT(!qFuzzyIsNull(pP.get<2>()));
    ret /= pP.get<2>();
    return ret;
}

/*!
 * Returns the focal length, i.e. the distance between source and detector in units of the
 * pixel spacing in x and y direction.
 */
Matrix<2, 1> ProjectionMatrix::focalLength() const
{
    auto K = intrinsicMatK();
    return { { K.get<0, 0>(), K.get<1, 1>() } };
}

/*!
 * Returns the skew coefficient, which is zero if the detector coordinate system is orthogonal.
 */
double ProjectionMatrix::skewCoefficient() const { return intrinsicMatK().get<0, 1>(); }

/*!
 * Maps a point in (cartesian) world coordinates onto the detector/image plane. This 2-dimensional
 * point (in pixel coordinates) will be returned by the function. The mapping is performed by
 * multiplying the projection matrix (from the left hand side) to the 4d vector [x, y, z, 1] and
 * afterwards the resulting homogeneous 3d vector is normalized back to cartesian coordinates (so
 * that the 3rd element is 1) and only the first two elements are returned.
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
 * Then, the return value is
 * \f$\left[\begin{array}{c}
 * x\\
 * y
 * \end{array}\right]=\left[\begin{array}{c}
 * \frac{\tilde{x}}{w}\\
 * \frac{\tilde{y}}{w}
 * \end{array}\right]\f$.
 *
 * \sa projectOntoDetector(const Matrix<3, 1>& worldCoordinate)
 */
Matrix<2, 1> ProjectionMatrix::projectOntoDetector(double X, double Y, double Z) const
{
    Matrix<2, 1> ret;
    const double w = X * get<2, 0>() + Y * get<2, 1>() + Z * get<2, 2>() + get<2, 3>();
    ret.get<0>() = ( X * get<0, 0>() + Y * get<0, 1>() + Z * get<0, 2>() + get<0, 3>() ) / w;
    ret.get<1>() = ( X * get<1, 0>() + Y * get<1, 1>() + Z * get<1, 2>() + get<1, 3>() ) / w;
    return ret;
}

/*!
 * This function does the same as projectOntoDetector(double X, double Y, double Z).
 */
Matrix<2, 1> ProjectionMatrix::projectOntoDetector(const Matrix<3, 1>& worldCoordinate) const
{
    return projectOntoDetector(worldCoordinate.get<0>(),
                               worldCoordinate.get<1>(),
                               worldCoordinate.get<2>());
}

} // namespace mat
} // namespace CTL
