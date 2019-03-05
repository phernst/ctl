#include "geometrydecoder.h"

#include "components/allgenerictypes.h"
#include "acquisition/preparesteps.h"

namespace CTL {

/*!
 * Constructs a GeometryDecoder object.
 */
GeometryDecoder::GeometryDecoder(const QSize &pixelPerModule, const QSizeF &pixelDimensions)
    : _pixelPerModule(pixelPerModule)
    , _pixelDimensions(pixelDimensions)
{
}

/*!
 * See
 * decodeFullGeometry(const FullGeometry& geometry, const QSize& pixelPerModule, const QSizeF& pixelDimensions).
 * It uses the internal `_pixelPerModule` and `_pixelDimensions` of the object.
 */
AcquisitionSetup GeometryDecoder::decodeFullGeometry(const FullGeometry& geometry) const
{
    return decodeFullGeometry(geometry, _pixelPerModule, _pixelDimensions);
}

SimpleCTsystem
GeometryDecoder::decodeSingleViewGeometry(const SingleViewGeometry& singleViewGeometry,
                                          const QSize& pixelPerModule,
                                          GeometryDecoder::PhysicalDimension physicalDimension,
                                          double mm)
{
    const auto referencePmat = singleViewGeometry.first();
    mat::Matrix<2, 1> focalLength = referencePmat.focalLength();

    QSizeF pixelDimensions;
    switch(physicalDimension)
    {
    case PhysicalDimension::PixelWidth:
        pixelDimensions.setWidth(mm);
        pixelDimensions.setHeight(mm * focalLength.get<0>() / focalLength.get<1>());
        break;
    case PhysicalDimension::PixelHeight:
        pixelDimensions.setWidth(mm * focalLength.get<1>() / focalLength.get<0>());
        pixelDimensions.setHeight(mm);
        break;
    case PhysicalDimension::SourceDetectorDistance:
        pixelDimensions.setWidth(mm / focalLength.get<0>());
        pixelDimensions.setHeight(mm / focalLength.get<1>());
        break;
    default:
        throw std::domain_error("invalid value for the physical dimension");
    }

    // X-ray source position
    auto srcPos = referencePmat.sourcePosition();

    // detector module locations
    QVector<mat::Location> moduleLocations = computeModuleLocations(
        singleViewGeometry, srcPos, pixelPerModule, pixelDimensions.width());

    // gantry contains only source location (no detector location)
    auto srcRot = computeSourceRotation(moduleLocations, srcPos);
    mat::Location sourceLocation = mat::Location(srcPos, srcRot);
    GenericGantry gantry(sourceLocation, mat::Location());

    // detector has full information about its location
    GenericDetector detector(pixelPerModule, pixelDimensions, std::move(moduleLocations));
    detector.setSkewCoefficient(referencePmat.skewCoefficient());

    return { std::move(detector), std::move(gantry), GenericSource() };
}

/*!
 * Returns matrix that rotates source perpendicular to the centroid of detector locations
 */
Matrix3x3 GeometryDecoder::computeSourceRotation(const QVector<mat::Location>& moduleLocations,
                                                 const Vector3x1& sourcePosition)
{
    // centroid of detector locations
    Vector3x1 centroid(0.0);
    for(const auto& modLoc : moduleLocations)
        centroid += modLoc.position;
    centroid /= moduleLocations.length();

    // vector from source to detector centroid
    Vector3x1 src2centroid = centroid - sourcePosition;
    src2centroid /= src2centroid.norm();

    // construct a rotation matrix that rotates the z-axis to `src2centroid` vector, i.e. it needs
    // to have the form
    // [ r1 r2 src2centroid ]
    // with r1 and r2 perpendicular to src2centroid

    // find axis that is as most as perpendicular to this vector
    uint axis = fabs(src2centroid.get<0>()) < fabs(src2centroid.get<1>()) ? 0 : 1;
    axis = fabs(src2centroid(axis)) < fabs(src2centroid.get<2>()) ? axis : 2;

    // init 1st vector with this axis
    Vector3x1 r1(0.0);
    r1(axis) = 1.0;

    // define canonical unit vectors
    mat::Matrix<1, 3> e1(1,0,0);
    mat::Matrix<1, 3> e2(0,1,0);
    mat::Matrix<1, 3> e3(0,0,1);

    // prepare cross product
    auto M = mat::vertcat(src2centroid.transposed(), r1.transposed());
    // 1st cross product
    r1 = { mat::det(mat::vertcat(e1,M)), mat::det(mat::vertcat(e2,M)), mat::det(mat::vertcat(e3,M)) };

    // prepare cross product
    M = mat::vertcat(src2centroid.transposed(), r1.transposed());
    // 2nd cross product
    Vector3x1 r2{ mat::det(mat::vertcat(e1,M)), mat::det(mat::vertcat(e2,M)), mat::det(mat::vertcat(e3,M)) };

    return mat::horzcat(r1, mat::horzcat(r2, src2centroid));
}

/*!
 * Decodes the set of projection matrices in \a geometry and constructs a GenericAcquisitionSetup
 * that represents all the geometry information that has been extracted.
 *
 * Besides projection matrices, information about the detector dimensions (i.e. number of pixels
 * in each module and their (physical) size) is required. The number of individual flat panel
 * modules is automatically readout from \a geometry.
 *
 * This method constructs a SimpleCTsystem consisting of a GenericSource, GenericDetector and a
 * GenericGantry component.
 *
 * Some remarks on system configuration: All source settings remain at default values (i.e. source
 * spectrum, focal spot size and focal spot position). Consider changing these afterwards, if
 * required. The full geometry information regarding the detector is stored in the location
 * specification of the individual detector modules. In particular, this means that the (global)
 * detector positioning - as queried for example by GenericGantry::detectorPosition() - will carry
 * no information (position defaults to (0,0,0) and rotation to identity matrix). Additionally, the
 * rotation of the source component cannot be determined without further information. Hence, it
 * remains at default value (i.e. identity matrix).
 */
AcquisitionSetup GeometryDecoder::decodeFullGeometry(const FullGeometry& geometry,
                                                     const QSize& pixelPerModule,
                                                     const QSizeF& pixelDimensions)
{
    // construct a generic system
    CTsystem theSystem;

    auto detector
        = makeComponent<GenericDetector>(pixelPerModule, pixelDimensions, QVector<mat::Location>());
    auto source = makeComponent<GenericSource>(QSizeF(0.0, 0.0), Vector3x1(0.0));
    auto gantry = makeComponent<GenericGantry>();

    theSystem << std::move(source) << std::move(detector) << std::move(gantry);

    AcquisitionSetup ret(theSystem);

    // extract geometry information
    for(const SingleViewGeometry& view : geometry)
    {
        auto srcPos = view.first().sourcePosition();
        auto moduleLocations = computeModuleLocations(view, srcPos, pixelPerModule,
                                                      pixelDimensions.width());

        // prepare steps gantry
        auto gantrySetter = std::make_shared<prepare::GenericGantryParam>();
        // note: all position/rotation information about detector stored in module locations
        gantrySetter->setDetectorLocation(mat::Location());
        auto srcRot = computeSourceRotation(moduleLocations, srcPos);
        gantrySetter->setSourceLocation(mat::Location(srcPos, srcRot));

        // detector prepare steps
        auto detectorSetter = std::make_shared<prepare::GenericDetectorParam>();
        detectorSetter->setModuleLocations(std::move(moduleLocations));

        // create AcquisitionSetup::View
        AcquisitionSetup::View viewSetting;
        viewSetting.setTimeStamp(ret.nbViews());
        viewSetting.addPrepareStep(gantrySetter);
        viewSetting.addPrepareStep(detectorSetter);

        ret.addView(viewSetting);
    }

    ret.prepareView(0);

    return ret;
}

/*!
 * Computes the physical center of the modules in world coordinates.
 */
QVector<mat::Location> GeometryDecoder::computeModuleLocations(const SingleViewGeometry& singleViewGeometry,
                                                               const Vector3x1& sourcePosition,
                                                               const QSize& pixelPerModule,
                                                               double pixelWidth)
{
    QVector<mat::Location> ret;
    ret.reserve(singleViewGeometry.length());

    for(const ProjectionMatrix& modulePmat : singleViewGeometry)
    {
        mat::Location modLoc;
        modLoc.position = modulePmat.directionSourceToPixel(0.5 * (pixelPerModule.width() - 1),
                                                            0.5 * (pixelPerModule.height() - 1),
                                                            mat::ProjectionMatrix::NormalizeByX);

        modLoc.position *= pixelWidth;     // scale to physical dimensions
        modLoc.position += sourcePosition; // offset by position of the source

        modLoc.rotation = modulePmat.rotationMatR();

        ret.append(modLoc);
    }

    return ret;
}

/*!
 * Returns the number of pixels per module of the detector that the geometry decoder shall assume
 * for the system.
 */
const QSize& GeometryDecoder::pixelPerModule() const
{
    return _pixelPerModule;
}

/*!
 * Returns the pixel dimensions of the detector that the geometry decoder shall assume for the
 * system.
 */
const QSizeF& GeometryDecoder::pixelDimensions() const
{
    return _pixelDimensions;
}

/*!
 * Sets the number of pixels per module of the detector that the geometry decoder shall assume for
 * the system to \a value.
 */
void GeometryDecoder::setPixelPerModule(const QSize& value)
{
    _pixelPerModule = value;
}

/*!
 * Sets the pixel dimensions of the detector that the geometry decoder shall assume for the system
 * to \a value.
 */
void GeometryDecoder::setPixelDimensions(const QSizeF& value)
{
    _pixelDimensions = value;
}

} // namespace CTL
