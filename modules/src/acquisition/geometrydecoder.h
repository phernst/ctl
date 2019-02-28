#ifndef GEOMETRYDECODER_H
#define GEOMETRYDECODER_H

#include "acquisitionsetup.h"
#include "fullgeometry.h"
#include "mat/mat.h"
#include <QSizeF>

namespace CTL {

/*!
 * \class GeometryDecoder
 *
 * \brief The GeometryDecoder class provides the functionality to extract the system configuration
 * from a set of projection matrices.
 *
 * This class can be used to generate an AcquisitionSetup that fully represents a set of given
 * projection matrices. Besides the projection matrices themselves, information about the detector
 * dimensions (i.e. number of pixels in each module and their (physical) size) is required.
 *
 * The projection matrices must be passed as a FullGeometry object. In doing so, the routine can
 * automatically readout the number of individual modules in the detector.
 */
class GeometryDecoder
{
public:
    enum PhysicalDimension { PixelWidth, PixelHeight, SourceDetectorDistance };

    GeometryDecoder(const QSize& pixelPerModule, const QSizeF& pixelDimensions);

    AcquisitionSetup decodeFullGeometry(const FullGeometry& geometry) const;

    const QSize& pixelPerModule() const;
    const QSizeF& pixelDimensions() const;
    void setPixelPerModule(const QSize& value);
    void setPixelDimensions(const QSizeF& value);

    static SimpleCTsystem decodeSingleViewGeometry(const SingleViewGeometry& singleViewGeometry,
                                                   const QSize& pixelPerModule,
                                                   PhysicalDimension physicalDimension = PixelWidth,
                                                   double mm = 1.0);

    static AcquisitionSetup decodeFullGeometry(const FullGeometry& geometry,
                                               const QSize& pixelPerModule,
                                               const QSizeF& pixelDimensions);

private:
    QSize _pixelPerModule; //!< Number of pixels of the assumed detector.
    QSizeF _pixelDimensions; //!< Pixel size of the assumed detector.

    static QVector<mat::Location> computeModuleLocations(const SingleViewGeometry& singleViewGeometry,
                                                         const Vector3x1& sourcePosition,
                                                         const QSize& pixelPerModule,
                                                         double pixelWidth);

    static Matrix3x3 computeSourceRotation(const QVector<mat::Location>& moduleLocations,
                                           const Vector3x1& sourcePosition);
};

} // namespace CTL

/*! \file */

#endif // GEOMETRYDECODER_H
