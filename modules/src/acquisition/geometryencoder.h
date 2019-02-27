#ifndef GEOMETRYENCODER_H
#define GEOMETRYENCODER_H

#include "acquisitionsetup.h"
#include "fullgeometry.h"
#include "mat/matrix_types.h"

#include <QSize>
#include <QVector>

namespace CTL {

// typedefs
typedef Vector3x1 Vector3x1WCS; //!< Alias name for 3D (column) vectors in world coordinates.
typedef Vector3x1 Vector3x1CTS; //!< Alias name for 3D (column) vectors in CT coordinates.

/*!
 * \class GeometryEncoder
 *
 * \brief The GeometryEncoder class translates the system geometry into a corresponding set of
 * projection matrices.
 *
 * This class provides the functionality to encode all geometry information about a particular
 * acquisition with a given imaging system into projection matrices. To do so, the entire
 * acquisition is sub-divided into two levels:
 * \li Individual views
 * \li Individual detector modules (i.e. separate flat panels)
 *
 * Within one particular view, all system parameters are fixed. That means, for example, that the
 * positioning of the detector system or the power settings of the X-ray source are constant.
 * For these constant conditions, the image acquisition is done with a detector system that
 * might consist of several, individually arranged flat panel modules. To describe this setting,
 * the entire system is described as a set of individual flat panel sub-systems (with a common
 * X-ray source). For each of these sub-systems, the geometry configuration can be fully described
 * by a projection matrix.
 * Across different views, system parameters may change. Usually, this involves a change in the
 * positioning of the components (like gantry rotations); but it may also contain variations in
 * other settings (e.g. dose modulation). Note that only geometric parameters are relevant for
 * encoding the system state into projection matrices.
 *
 * To encode the geometry for an entire acquisition, the method encodeFullGeometry() needs to be
 * called. It computes and returns the full set of projection matrices for all views (and all
 * modules). If only a single system configuration shall be encoded, the method
 * encodeSingleViewGeometry() can be used instead.
 */
class GeometryEncoder
{
public:
    GeometryEncoder(const SimpleCTsystem* system);

    void assignSystem(const SimpleCTsystem* system);
    SingleViewGeometry encodeSingleViewGeometry() const;
    const SimpleCTsystem* system() const;

    // static methods
    static FullGeometry encodeFullGeometry(AcquisitionSetup setup);
    static SingleViewGeometry encodeSingleViewGeometry(const SimpleCTsystem& system);

private:
    // methods
    Vector3x1WCS finalSourcePosition() const;

    static ProjectionMatrix computeIndividualModulePMat(const Vector3x1WCS& finalSourcePosition,
                                                        const Matrix3x3& detectorRotation,
                                                        const Matrix3x3& K);
    static Matrix3x3 intrinsicParameterMatrix(const Vector3x1CTS& principalPoint,
                                              const QSize& nbPixel,
                                              const QSizeF& pixelDimensions);

    // member variables
    const SimpleCTsystem* _system; //!< Pointer to system whose geometry shall be encoded.
};

} // namespace CTL

/*! \file */

#endif // GEOMETRYENCODER_H
