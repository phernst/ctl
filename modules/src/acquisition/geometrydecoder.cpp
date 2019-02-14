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

    QVector<mat::Location> moduleLocations;

    // extract geometry information
    for(const SingleViewGeometry& view : geometry)
    {
        auto srcPos = view.first().sourcePosition();

        // compute module locations
        moduleLocations.clear();
        for(const ProjectionMatrix& modulePmat : view)
        {
            mat::Location modLoc;
            modLoc.position = modulePmat.directionSourceToPixel(0.5 * (pixelPerModule.width() - 1),
                                                                0.5 * (pixelPerModule.height() - 1),
                                                                mat::ProjectionMatrix::NormalizeByX);

            modLoc.position *= pixelDimensions.width(); // scale to physical dimensions
            modLoc.position += srcPos;                  // offset by position of the source

            modLoc.rotation = modulePmat.rotationMatR();

            moduleLocations.append(modLoc);
        }

        auto gantrySetter = std::make_shared<prepare::GenericGantryParam>();
        // note: all position/rotation information about detector stored in module locations
        gantrySetter->setDetectorLocation(mat::Location());
        // note: source position cannot be extracted w.o. further information
        gantrySetter->setSourceLocation(mat::Location(srcPos, mat::eye<3>()));

        auto detectorSetter = std::make_shared<prepare::GenericDetectorParam>();
        detectorSetter->setModuleLocations(moduleLocations);

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
