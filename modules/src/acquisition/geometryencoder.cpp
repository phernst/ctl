#include "geometryencoder.h"
#include "components/allgenerictypes.h"

#include <iostream>

namespace CTL {

/*!
 * Constructs a GeometryEncoder object. This object refers to a SimpleCTSystem.
 *
 * Note that the SimpleCTSystem on which \a system points to is not owned by the GeometryEncoder
 * and the client have to make sure that the object \a system points to a valid object before
 * calling encodeSingleViewGeometry().
 */
GeometryEncoder::GeometryEncoder(const SimpleCTSystem* system)
    : _system(system)
{
}

/*!
 * Returns a pointer to the (constant) SimpleCTSystem that has been assigned to this instance.
 */
const SimpleCTSystem *GeometryEncoder::system() const
{
    return _system;
}

/*!
 * Returns the (average) effective pixel area [in mm²] of detector pixels in the detector module
 * \a module of this instance's system within its current state.
 *
 * \sa effectivePixelArea(const SimpleCTSystem& system, uint module).
 */
float GeometryEncoder::effectivePixelArea(uint module) const
{
    if(_system == nullptr)
        throw std::runtime_error("GeometryEncoder::effectivePixelArea(): No system has been set.");

    return effectivePixelArea(*_system, module);
}

/*!
 * Returns the (average) effective pixel areas of all individual modules in \a system.
 *
 * \sa effectivePixelArea(const SimpleCTSystem& system, uint module).
 */
std::vector<float> GeometryEncoder::effectivePixelAreas() const
{
    if(_system == nullptr)
        throw std::runtime_error("GeometryEncoder::effectivePixelAreas(): No system has been set.");

    return effectivePixelAreas(*_system);
}

/*!
 * Assignes \a system to this instance.
 *
 * This instance does not take ownership of \a system.
 */
void GeometryEncoder::assignSystem(const SimpleCTSystem* system)
{
    _system = system;
}

/*!
 * Computes a SingleViewGeometry based on the state of the internal SimpleCTSystem.
 */
SingleViewGeometry GeometryEncoder::encodeSingleViewGeometry() const
{
    if(_system == nullptr)
        return {};

    return encodeSingleViewGeometry(*_system);
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
SingleViewGeometry GeometryEncoder::encodeSingleViewGeometry(const SimpleCTSystem& system)
{
    SingleViewGeometry ret;

    AbstractDetector* detector = system.detector();
    ret.reserve(detector->nbDetectorModules());

    const Vector3x1WCS sourcePos = finalSourcePosition(system);
    const Vector3x1WCS detectorPos = system.gantry()->detectorPosition();
    const Matrix3x3 detectorRot = system.gantry()->detectorRotation();
    const Matrix3x3 detectorRot_T = detectorRot.transposed();

    auto pixelDim = detector->pixelDimensions();
    auto moduleSize = detector->nbPixelPerModule();
    auto moduleLocs = detector->moduleLocations();
    auto skewCoeff = detector->skewCoefficient();

    for(uint module = 0, nbModules = moduleLocs.count(); module < nbModules; ++module)
    {
        auto modLoc = moduleLocs.at(module);

        Vector3x1WCS modulePos = detectorPos + detectorRot_T * modLoc.position;
        Matrix3x3 totalRot = modLoc.rotation * detectorRot;

        Vector3x1CTS pPointDeviation = totalRot * (modulePos - sourcePos);
        Matrix3x3 K = intrinsicParameterMatrix(pPointDeviation, moduleSize, pixelDim, skewCoeff);

        ret.append(computeIndividualModulePMat(sourcePos, totalRot, K));
    }

    return ret;
}

/*!
 * Returns the (average) effective pixel areas of all individual modules in \a system.
 *
 * \sa effectivePixelArea(const SimpleCTSystem& system, uint module).
 */
std::vector<float> GeometryEncoder::effectivePixelAreas(const SimpleCTSystem &system)
{
    auto nbMod = system.detector()->nbDetectorModules();
    std::vector<float> areas(nbMod);
    for(uint mod = 0; mod < nbMod; ++mod)
        areas[mod] = effectivePixelArea(system, mod);

    return areas;
}

/*!
 * Returns the final position of detector \a module in \a system.
 *
 * \sa finalModulePosition(const SimpleCTSystem &system, uint module).
 */
Vector3x1WCS GeometryEncoder::finalModulePosition(uint module) const
{
    if(_system == nullptr)
        throw std::runtime_error("GeometryEncoder::finalModulePosition(): No system has been set.");

    return finalModulePosition(*_system, module);
}

/*!
 * Returns the final rotation of detector \a module in \a system.
 *
 * \sa finalModuleRotation(const SimpleCTSystem &system, uint module).
 */
Matrix3x3 GeometryEncoder::finalModuleRotation(uint module) const
{
    if(_system == nullptr)
        throw std::runtime_error("GeometryEncoder::finalModuleRotation(): No system has been set.");

    return finalModuleRotation(*_system, module);
}

/*!
 * Computes the final position of the origin of the X-rays. This takes into account the location
 * of the source component itself as well as the positioning of the focal spot.
 *
 * \sa finalSourcePosition(const SimpleCTSystem& system).
 */
Vector3x1WCS GeometryEncoder::finalSourcePosition() const
{
    if(_system == nullptr)
        throw std::runtime_error("GeometryEncoder::finalSourcePosition(): No system has been set.");

    return finalSourcePosition(*_system);
}

// ##############
// static methods
// ##############

/*!
 * Returns the (average) effective pixel area [in mm²] of detector pixels in the detector module
 * \a module of the detector in \a system within its current state.
 *
 * Effective pixel area refers to the (normal) area that a pixel would expose (from
 * the source point of view) if it were placed at a distance of 1 meter from the source.
 * It computes as follows:
 *
 * \f$
 * A_{m}^{\textrm{eff}}=A^{\textrm{nom}}\cdot\phi_{m}\cdot
 * \left(\frac{1\,\textrm{m}}{d_{m}}\right)^{2},
 * \qquad m=\mathtt{module}\in\left[0,\textrm{nbModules}-1\right]\\
 * \begin{align*}
 * \textrm{with:}\;A^{\textrm{nom}} & =s_{x}\cdot s_{y}\\
 * d_{m} & =\left\Vert \mathbf{r}_{m}^{\textrm{src - det.mod.}}\right\Vert \\
 * \phi_{m} & =\left\langle \mathbf{\hat{r}}_{m}^{\textrm{src - det.mod.}},
 * \hat{\mathbf{r}}_{m}^{\textrm{src - princ.pt.}}\right\rangle \\
 * \hat{\mathbf{r}}^{(\cdot)} & :=\mathbf{r}^{(\cdot)}/\left\Vert \mathbf{r}^{(\cdot)}\right\Vert
 * \end{align*}
 * \f$
 *
 * Here, \f$A^{\textrm{nom}}\f$ denotes the nominal area of an individual detector pixel (i.e. pixel
 * width \f$s_{x}\f$ times heigth \f$s_{y}\f$), \f$d_{m}\f$ is the distance (in meters) from the
 * source to the position of
 * detector module \a m, and \f$\phi_{m}\f$ refers to the cosine of the angle between the normal
 * vector of the module \a m and the connection line between source and the module's position.
 * Due to the fact that a constant distance \f$d_{m}\f$ and orientation \f$\phi_{m}\f$ is assumed
 * for all pixels in the module, this computation provides an approximation of the average
 * effective pixel area for the individual pixels in that module. The assumption can be violated in
 * case of very small distances, large modules, and/or large angulations.
 *
 * Note that the source position used in these computations is the final position, i.e. including
 * focal spot position shifts and/or displacements of the source component.
 */
float GeometryEncoder::effectivePixelArea(const SimpleCTSystem& system, uint module)
{
    auto nominalArea = system.detector()->pixelDimensions().width() *
            system.detector()->pixelDimensions().height();

    auto sourceToModuleVector = finalModulePosition(system, module) - finalSourcePosition(system);

    auto distance = sourceToModuleVector.norm();
    sourceToModuleVector /= distance;
    distance *= 1.0e-3; // in meters

    auto scalarProd = finalModuleRotation(system, module).row<2>() * sourceToModuleVector;

    return static_cast<float>(nominalArea * scalarProd / (distance * distance));
}

/*!
 * Returns the final position of detector \a module in \a system.
 *
 * Computes as:
 *
 * \f$
 * t_{\textrm{module}}^{\textrm{final}}=t_{\textrm{det}}^{\textrm{final}}+
 * \left(R_{\textrm{det}}^{\textrm{total}}\right)^{T}\cdot t_{\textrm{module}}
 * \f$
 */
Vector3x1WCS GeometryEncoder::finalModulePosition(const SimpleCTSystem &system, uint module)
{
    const auto& detectorRot = system.gantry()->detectorRotation();
    const auto& modLoc = system.detector()->moduleLocation(module);

    return system.gantry()->detectorPosition() + detectorRot.transposed() * modLoc.position;
}

/*!
 * Returns the final rotation of detector module \a module in \a system.
 *
 * Computes as:
 *
 * \f$
 * R_{\textrm{module}}^{\textrm{final}}=R_{\textrm{module}}\cdot R_{\textrm{det}}^{\textrm{total}}
 * \f$
 */
Matrix3x3 GeometryEncoder::finalModuleRotation(const SimpleCTSystem &system, uint module)
{
    return system.detector()->moduleLocation(module).rotation * system.gantry()->detectorRotation();
}

/*!
 * Computes the final position of the origin of the X-rays. This takes into account the location
 * of the source component itself as well as the positioning of the focal spot.
 *
 * Computes as:
 *
 * \f$
 * t_{\textrm{source}}^{\textrm{final}}=t_{\textrm{source}}+
 * R_{\textrm{source}}^{\textrm{total}}\cdot t_{\textrm{focal spot}}
 * \f$
 */
Vector3x1WCS GeometryEncoder::finalSourcePosition(const SimpleCTSystem& system)
{
    return system.gantry()->sourcePosition()
            + system.gantry()->sourceRotation() * system.source()->focalSpotPosition();
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
 * Computes the intrinsic parameter matrix from the \a principalPoint, the number of pixels in the
 * detector (module) \a nbPixel and the dimensions of the pixels \a pixelDimensions.
 */
Matrix3x3 GeometryEncoder::intrinsicParameterMatrix(const Vector3x1CTS& principalPointDeviation,
                                                    const QSize& nbPixel,
                                                    const QSizeF& pixelDimensions,
                                                    double skew)
{
    Q_ASSERT(pixelDimensions.width() != 0.0);
    Q_ASSERT(pixelDimensions.height() != 0.0);

    // extract z component for focal length
    auto focalLengthMM = std::fabs(principalPointDeviation.get<2>());
    // convert mm in pixel
    auto fX = focalLengthMM / pixelDimensions.width();
    auto fY = focalLengthMM / pixelDimensions.height();

    Q_ASSERT(fY != 0.0);

    // principal point: mounting point in CTS "(N-1)/2" minus the
    //                  deviation of source-to-mounting-point vector from z-axis (pinc. ray) in CTS
    //           x S
    //          /|
    //      M-S/ |z-axis
    //        /  |
    // ------<---|-- detector
    //       ^ ^ ^
    //       M d P
    //
    // S - x-ray source
    // P - principal point
    // M - mounting point
    // d - x-y part of "M-S" in the CTS frame, i.e. the
    //     deviation of source-to-mounting-point vector from z-axis (pincipal ray)

    // convert mm in pixel
    auto pPDevPixelX = principalPointDeviation.get<0>() / pixelDimensions.width();
    auto pPDevPixelY = principalPointDeviation.get<1>() / pixelDimensions.height();

    // skew correction for x coordinate
    pPDevPixelX += pPDevPixelY * skew / fY;

    // mounting point in the CTL is alway the physical center of the detector module
    auto mountingX = 0.5 * double(nbPixel.width() - 1);
    auto mountingY = 0.5 * double(nbPixel.height() - 1);

    auto principalPointX = mountingX - pPDevPixelX;
    auto principalPointY = mountingY - pPDevPixelY;

    return Matrix3x3(fX , skew, principalPointX,
                     0.0, fY,   principalPointY,
                     0.0, 0.0,  1.0);
}

} // namespace CTL
