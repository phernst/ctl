#include "geometryencoder.h"
#include "components/allgenerictypes.h"

#include <iostream>

namespace CTL {

/*!
 * Constructs a GeometryEncoder object. This object refers to a SimpleCTsystem.
 *
 * Note that the SimpleCTsystem on which \a system points to is not owned by the GeometryEncoder
 * and the client have to make sure that the object \a system points to a valid objec before calling
 * encodeSingleViewGeometry().
 */
GeometryEncoder::GeometryEncoder(const SimpleCTsystem* system)
    : _system(system)
{
}

/*!
 * Returns a pointer to the (constant) SimpleCTsystem that has been assigned to this instance.
 */
const SimpleCTsystem *GeometryEncoder::system() const
{
    return _system;
}

/*!
 * Assignes \a system to this instance.
 *
 * This instance does not take ownership of \a system.
 */
void GeometryEncoder::assignSystem(const SimpleCTsystem *system)
{
    _system = system;
}

/*!
 * Computes a SingleViewGeometry based on the state of the internal SimpleCTsystem.
 */
SingleViewGeometry GeometryEncoder::encodeSingleViewGeometry() const
{
    if(_system == nullptr)
        return {};

    SingleViewGeometry ret;

    AbstractDetector* detector = _system->detector();
    ret.reserve(detector->nbDetectorModules());

    Vector3x1WCS sourcePos = finalSourcePosition();
    Vector3x1WCS detectorPos = _system->gantry()->detectorPosition();
    Matrix3x3 detectorRot = _system->gantry()->detectorRotation();

    auto pixelDim = detector->pixelDimensions();
    auto moduleSize = detector->nbPixelPerModule();
    auto moduleLocs = detector->moduleLocations();

    for(uint module = 0; module < detector->nbDetectorModules(); ++module)
    {
        auto modLoc = moduleLocs.at(module);

        Vector3x1WCS modulePos = detectorPos + detectorRot.transposed() * modLoc.position;
        Matrix3x3 moduleRot = modLoc.rotation;

        Vector3x1CTS pPoint = principalPoint(modulePos - sourcePos, moduleRot * detectorRot);
        Matrix3x3 K = intrinsicParameterMatrix(pPoint, moduleSize, pixelDim);

        ret.append(computeIndividualModulePMat(sourcePos, moduleRot * detectorRot, K));
    }

    return ret;
}

/*!
 * Computes and returns the geometry representation of \a setup as a set of projection matrices.
 *
 * The conceptual work flow is as follows:
 *
 * \htmlinclude pseudo_geometryEncoder.html
 */
FullGeometry GeometryEncoder::encodeFullGeometry(AcquisitionSetup setup)
{
    const auto nbViews = setup.nbViews();
    FullGeometry ret;
    ret.reserve(nbViews);

    GeometryEncoder geoEncoder(setup.system());

    for(uint view = 0; view < nbViews; ++view)
    {
        setup.prepareView(view);
        ret.append(geoEncoder.encodeSingleViewGeometry());
    }

    return ret;
}

/*!
 * Computes the geometry representation for a single view with the current configuration of
 * \a system.
 */
SingleViewGeometry GeometryEncoder::encodeSingleViewGeometry(const SimpleCTsystem& system)
{
    GeometryEncoder geoEncoder(&system);

    return geoEncoder.encodeSingleViewGeometry();
}

/*!
 * Computes the final position of the origin of the X-rays. This takes into account the location
 * of the source component itself as well as the positioning of the focal spot.
 */
Vector3x1WCS GeometryEncoder::finalSourcePosition() const
{
    return _system->gantry()->sourcePosition()
        + _system->gantry()->sourceRotation() * _system->source()->focalSpotPosition();
}

/*!
 * Computes the geometry representation for a single detector module with an intrinsic parameter
 * matrix \a K, located at position \a detectorPosition given a source located at \a sourcePosition.
 */
ProjectionMatrix GeometryEncoder::computeIndividualModulePMat(const Vector3x1WCS& sourcePosition,
                                                              const Matrix3x3& detectorRotation,
                                                              const Matrix3x3& K)
{
    auto pMat = ProjectionMatrix::compose(K, detectorRotation, sourcePosition);
    return pMat.normalized();
}

/*!
 * Computes the principal point for a detector with the vector connecting source and detector center
 * \a sourceToDetectorVector and the transformation from world to CT coordinates \a rotation.
 *
 * Same as \a rotation * \a sourceToDetectorVector.
 */
Vector3x1CTS GeometryEncoder::principalPoint(const Vector3x1WCS& sourceToDetectorVector,
                                             const Matrix3x3& rotation)
{
    Vector3x1CTS principPt = (rotation * sourceToDetectorVector); // transformed center of detector

    return principPt;
}

/*!
 * Computes the intrinsic parameter matrix from the \a principalPoint, the number of pixels in the
 * detector (module) \a nbPixel and the dimensions of the pixels \a pixelDimensions.
 */
Matrix3x3 GeometryEncoder::intrinsicParameterMatrix(const Vector3x1CTS& principalPoint,
                                                    const QSize& nbPixel,
                                                    const QSizeF& pixelDimensions)
{
    double focalLengthMM = fabs(principalPoint(2));
    double principalPointS
        = -principalPoint.get<0>() / pixelDimensions.width() + 0.5 * double(nbPixel.width() - 1);
    double principalPointT
        = -principalPoint.get<1>() / pixelDimensions.height() + 0.5 * double(nbPixel.height() - 1);

    return Matrix3x3(focalLengthMM / pixelDimensions.width(), 0.0, principalPointS,
                     0.0, focalLengthMM / pixelDimensions.height(), principalPointT,
                     0.0, 0.0, 1.0);
}

} // namespace CTL
