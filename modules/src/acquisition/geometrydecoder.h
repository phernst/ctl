#ifndef CTL_GEOMETRYDECODER_H
#define CTL_GEOMETRYDECODER_H

#include "acquisitionsetup.h"
#include "viewgeometry.h"
#include "mat/mat.h"
#include <QSizeF>
#include <QPair>

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
    GeometryDecoder(const QSize& pixelPerModule,
                    PhysicalDimension physicalDimension = PixelWidth,
                    double mm = 1.0);

    AcquisitionSetup decodeFullGeometry(const FullGeometry& geometry) const;

    const QPair<PhysicalDimension, double>& dimensionReference() const;
    const QSize& pixelPerModule() const;
    const QSizeF& pixelDimensions() const;
    void setDimensionReference(PhysicalDimension physicalDimension, double mm);
    void setPixelPerModule(const QSize& value);
    void setPixelDimensions(const QSizeF& value);

    static SimpleCTSystem decodeSingleViewGeometry(const SingleViewGeometry& singleViewGeometry,
                                                   const QSize& pixelPerModule,
                                                   PhysicalDimension physicalDimension = PixelWidth,
                                                   double mm = 1.0);

    static AcquisitionSetup decodeFullGeometry(const FullGeometry& geometry,
                                               const QSize& pixelPerModule,
                                               const QSizeF& pixelDimensions);

    static AcquisitionSetup decodeFullGeometry(const FullGeometry& geometry,
                                               const QSize& pixelPerModule,
                                               PhysicalDimension physicalDimension = PixelWidth,
                                               double mm = 1.0);

private:
    QSize _pixelPerModule; //!< Number of pixels of the assumed detector.
    QSizeF _pixelDimensions; //!< Pixel size of the assumed detector.
    QPair<PhysicalDimension, double> _physicalDimensionReference = { PixelWidth, 1.0 };

    static QSizeF computePixelSize(const mat::ProjectionMatrix& pMat,
                                   PhysicalDimension physicalDimension,
                                   double mm);

    static QVector<mat::Location> computeModuleLocations(const SingleViewGeometry& singleViewGeometry,
                                                         const Vector3x1& sourcePosition,
                                                         const QSize& pixelPerModule,
                                                         double pixelWidth);

    static Matrix3x3 computeSourceRotation(const QVector<mat::Location>& moduleLocations,
                                           const Vector3x1& sourcePosition);
};

} // namespace CTL

/*! \file */

#endif // CTL_GEOMETRYDECODER_H
