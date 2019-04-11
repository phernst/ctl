#include "tubulargantry.h"

#include <cmath>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(TubularGantry)

/*!
 * Constructs a TubularGantry with the dimensions \a sourceToDetectorDistance and \a
 * sourceToIsoCenterDistance. Optionally, the system configuration can be specified by \a
 * rotationAngle, \a pitchPosition and \a tiltAngle. If unspecified, these values default to zero.
 */
TubularGantry::TubularGantry(double sourceToDetectorDistance,
                             double sourceToIsoCenterDistance,
                             double rotationAngle,
                             double pitchPosition,
                             double tiltAngle,
                             const QString& name)
    : AbstractGantry(name)
    , _sourceToDetectorDistance(sourceToDetectorDistance)
    , _sourceToIsoCenterDistance(sourceToIsoCenterDistance)
    , _rotationAngle(rotationAngle)
    , _pitchPosition(pitchPosition)
    , _tiltAngle(tiltAngle)
{
}

/*!
 * Convenience constructor that constructs a TubularGantry similar to the full constructor using the
 * dimensions \a sourceToDetectorDistance and \a sourceToIsoCenterDistance, but still allows to
 * specify a custom name for the component when omitting the system configuration. Here, all
 * configuration parameters (i.e. gantry rotation angle, table pitch position and gantry tilt angle)
 * default to zero.
 */
TubularGantry::TubularGantry(double sourceToDetectorDistance,
                             double sourceToIsoCenterDistance,
                             const QString& name)
    : TubularGantry(sourceToDetectorDistance, sourceToIsoCenterDistance, 0.0, 0.0, 0.0, name)
{
}

/*!
 * Returns the nominal detector location. This ignores any (optional) detector or gantry displacement.
 *
 * Overrides the base class method and computes the detector location based on the tubular gantry
 * parametrization (i.e. source-to-detector distance etc.).
 *
 * \f$
 * L_{\textrm{det}}^{\textrm{nominal}}=
 * (t_{\textrm{det}}^{\textrm{nominal}},R_{\textrm{det}}^{\textrm{nominal}}),
 * \f$
 *
 * where:
 *
 * \f$
 * \begin{align*}
 * t_{\textrm{det}}^{\textrm{nominal}} & =R_{\textrm{gantry}}\cdot\left[\begin{array}{c}
 * \mathtt{SID}-\mathtt{SDD}\\0\\-\mathtt{pitchPos}\end{array}\right]=
 * \mathbf{R}_{x}(\mathtt{tilt})\cdot\mathbf{R}_{z}(\mathtt{gantryRot})
 * \cdot\left[\begin{array}{c}
 * \mathtt{SID}\\0\\-\mathtt{pitchPos}\end{array}\right]\\
 * R_{\textrm{det}}^{\textrm{nominal}} & =\mathbf{R}_{x}(\mathtt{\pi/2})
 * \cdot\mathbf{R}_{z}(-\pi/2)\cdot R_{\textrm{gantry}}^{T}
 * \end{align*}
 * \f$
 */
mat::Location TubularGantry::nominalDetectorLocation() const { return detectorLocationTG(); }

/*!
 * Returns the nominal source location. This ignores any (optional) source or gantry displacement.
 *
 * Overrides the base class method and computes the source location based on the tubular gantry
 * parametrization (i.e. source-to-detector distance etc.).
 *
 * \f$
 * L_{\textrm{src}}^{\textrm{nominal}}=
 * (t_{\textrm{src}}^{\textrm{nominal}},R_{\textrm{src}}^{\textrm{nominal}}),
 * \f$
 *
 * where:
 *
 * \f$
 * \begin{align*}
 * t_{\textrm{src}}^{\textrm{nominal}} & =R_{\textrm{gantry}}\cdot\left[\begin{array}{c}
 * \mathtt{SID}\\0\\-\mathtt{pitchPos}\end{array}\right]=
 * \mathbf{R}_{x}(\mathtt{tilt})\cdot\mathbf{R}_{z}(\mathtt{gantryRot})
 * \cdot\left[\begin{array}{c}
 * \mathtt{SID}\\0\\-\mathtt{pitchPos}\end{array}\right]\\
 * R_{\textrm{src}}^{\textrm{nominal}} & =R_{\textrm{gantry}}
 * \cdot\mathbf{R}_{z}(\pi/2)\cdot\mathbf{R}_{x}(\mathtt{-\pi/2})
 * \end{align*}
 * \f$
 */
mat::Location TubularGantry::nominalSourceLocation() const { return sourceLocationTG(); }

/*!
 * Computed the current source location (i.e. position and rotation) based on the system
 * configuration.
 *
 * Same as mat::Location(sourcePositionTG(), sourceRotationTG())
 */
mat::Location TubularGantry::sourceLocationTG() const
{
    return { sourcePositionTG(), sourceRotationTG() };
}

/*!
 * Computed the current detector location (i.e. position and rotation) based on the system
 * configuration.
 *
 * Same as mat::Location(detectorPositionTG(), detectorRotationTG())
 */
mat::Location TubularGantry::detectorLocationTG() const
{
    return { detectorPositionTG(), detectorRotationTG() };
}

/*!
 * Computes the current source position based on the system configuration.
 *
 * \f$
 * t_{\textrm{src}}^{\textrm{nominal}}=R_{\textrm{gantry}}\cdot\left[\begin{array}{c}
 * \mathtt{SID}\\0\\-\mathtt{pitchPos}\end{array}\right]=
 * \mathbf{R}_{x}(\mathtt{tilt})\cdot\mathbf{R}_{z}(\mathtt{gantryRot})
 * \cdot\left[\begin{array}{c}
 * \mathtt{SID}\\0\\-\mathtt{pitchPos}\end{array}\right]
 * \f$
 */
Vector3x1 TubularGantry::sourcePositionTG() const
{
    Vector3x1 pos({ _sourceToIsoCenterDistance, 0.0, -_pitchPosition });

    return totalGantryRotation() * pos;
}

/*!
 * Computes the current source rotation based on the system configuration.
 *
 * This is computed such that the gantry rotation angle is defined with respect to the (WCS) x-axis.
 * Hence, for an angle of zero degrees, the source is located on the positive x-axis.
 *
 * \f$
 * R_{\textrm{src}}^{\textrm{nominal}}=R_{\textrm{gantry}}
 * \cdot\mathbf{R}_{z}(\pi/2)\cdot\mathbf{R}_{x}(\mathtt{-\pi/2})
 * \f$
 */
Matrix3x3 TubularGantry::sourceRotationTG() const
{
    return totalGantryRotation() * mat::rotationMatrix(M_PI_2, Qt::ZAxis)
        * mat::rotationMatrix(-M_PI_2, Qt::XAxis);
}

/*!
 * Computes the current detector position based on the system configuration.
 *
 * \f$
 * t_{\textrm{det}}^{\textrm{nominal}}=R_{\textrm{gantry}}\cdot\left[\begin{array}{c}
 * \mathtt{SID}-\mathtt{SDD}\\0\\-\mathtt{pitchPos}\end{array}\right]=
 * \mathbf{R}_{x}(\mathtt{tilt})\cdot\mathbf{R}_{z}(\mathtt{gantryRot})
 * \cdot\left[\begin{array}{c}
 * \mathtt{SID}\\0\\-\mathtt{pitchPos}\end{array}\right]
 * \f$
 */
Vector3x1 TubularGantry::detectorPositionTG() const
{
    Vector3x1 pos(
        { -(_sourceToDetectorDistance - _sourceToIsoCenterDistance), 0.0, -_pitchPosition });

    return totalGantryRotation() * pos;
}

/*!
 * Computes the current detector rotation based on the system configuration.
 *
 * This is computed such that the gantry rotation angle is defined with respect to the (WCS) x-axis.
 * Hence, for an angle of zero degrees, the detector is located on the negative x-axis.
 *
 * \f$
 * R_{\textrm{det}}^{\textrm{nominal}}=\mathbf{R}_{x}(\mathtt{\pi/2})
 * \cdot\mathbf{R}_{z}(-\pi/2)\cdot R_{\textrm{gantry}}^{T}
 * \f$
 */
Matrix3x3 TubularGantry::detectorRotationTG() const
{
    Matrix3x3 rotMat = totalGantryRotation() * mat::rotationMatrix(M_PI_2, Qt::ZAxis)
        * mat::rotationMatrix(-M_PI_2, Qt::XAxis); // active form

    return rotMat.transposed(); // passive form
}

/*!
 * Computes the total rotation matrix of the gantry system. This includes gantry rotation and tilt:
 *
 * \f$
 * R_{\textrm{gantry}}=
 * \mathbf{R}_{x}(\mathtt{tilt})\cdot\mathbf{R}_{z}(\mathtt{gantryRot})
 * \f$
 */
Matrix3x3 TubularGantry::totalGantryRotation() const
{
    Matrix3x3 rotZ = mat::rotationMatrix(_rotationAngle, Qt::ZAxis);
    Matrix3x3 tilt = mat::rotationMatrix(_tiltAngle, Qt::XAxis);

    return tilt * rotZ;
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base classes (GenericGantry and GenericComponent), the
 * info string contains the following details:
 * \li Source-to-detector distance
 * \li Source-to-iso-center distance
 * \li Rotation angle
 * \li Table pitch position
 * \li Tilt angle.
 */
QString TubularGantry::info() const
{
    QString ret(AbstractGantry::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tSource-to-detector distance: " + QString::number(_sourceToDetectorDistance) + " mm\n" +
        "\tSource-to-iso-center distance: " + QString::number(_sourceToIsoCenterDistance) + " mm\n" +
        "\tRotation angle: " + QString::number(_rotationAngle * 180.0 / M_PI) + " deg\n" +
        "\tTable pitch position: " + QString::number(_pitchPosition) + " mm\n" +
        "\tTilt angle: " + QString::number(_tiltAngle * 180.0 / M_PI) + " deg\n";

    ret += (this->type() == TubularGantry::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Returns the default name for the component: "Tubular gantry".
 */
QString TubularGantry::defaultName()
{
    static const QString defName(QStringLiteral("Tubular gantry"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void TubularGantry::fromVariant(const QVariant& variant)
{
    AbstractGantry::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _sourceToDetectorDistance = varMap.value("source-detector distance").toDouble();
    _sourceToIsoCenterDistance = varMap.value("source-isocenter distance").toDouble();
    _rotationAngle = varMap.value("rotation angle").toDouble();
    _pitchPosition = varMap.value("pitch position").toDouble();
    _tiltAngle = varMap.value("tilt angle").toDouble();
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant TubularGantry::toVariant() const
{
    QVariantMap ret = AbstractGantry::toVariant().toMap();

    ret.insert("source-detector distance", _sourceToDetectorDistance);
    ret.insert("source-isocenter distance", _sourceToIsoCenterDistance);
    ret.insert("rotation angle", _rotationAngle);
    ret.insert("pitch position", _pitchPosition);
    ret.insert("tilt angle", _tiltAngle);

    return ret;
}

// use documentation of GenericComponent::clone()
SystemComponent* TubularGantry::clone() const { return new TubularGantry(*this); }

/*!
 * Sets the table pitch to \a position. Position value is expected in millimeters.
 */
void TubularGantry::setPitchPosition(double position) { _pitchPosition = position; }

/*!
 * Sets the gantry rotation to \a angle. Rotation angle is expected in radians.
 */
void TubularGantry::setRotationAngle(double angle) { _rotationAngle = angle; }

/*!
 * Sets the gantry tilt to \a angle. Tilt angle is expected in radians.
 */
void TubularGantry::setTiltAngle(double angle) { _tiltAngle = angle; }

} // namespace CTL
