#include "carmgantry.h"
#include "mat/matrix_utils.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(CarmGantry)

/*!
 * Constructs a CarmGantry with a C-arm span (i.e. source-to-detector distance) of \a cArmSpan and
 * \a name. If not explicitly specified, the C-arm span defaults to 1000 mm.
 */
CarmGantry::CarmGantry(double cArmSpan, const QString &name)
    : AbstractGantry(name),
      _cArmSpan(cArmSpan)
{
}

/*!
 * Returns the nominal detector location. This ignores any (optional) detector displacement.
 *
 * Overrides the base class method and computes the detector location based on the C-arm
 * parametrization (i.e. source location and C-arm span).
 */
mat::Location CarmGantry::nominalDetectorLocation() const { return detectorLocationCA(); }

/*!
 * Returns the nominal source location. This ignores any (optional) source displacement.
 *
 * Overrides the base class method and returns the source location like specified as part of the
 * C-arm parametrization (i.e. source location and C-arm span).
 *
 * \sa location().
 */
mat::Location CarmGantry::nominalSourceLocation() const { return _location; }

/*!
 * Computes the detector position based on the location of the source and the C-arm span, based on
 * the assumption that all components are aligned on the optical axis.
 *
 * \f$
 * t_{\textrm{det}}^{\textrm{nominal}}=t_{\textrm{C-arm}}-R_{\textrm{C-arm}}
 * \cdot\left[\begin{array}{c}0\\0\\-\mathtt{cArmSpan}\end{array}\right],
 * \f$
 *
 * where \f$t_{\textrm{C-arm}}\f$ and \f$R_{\textrm{C-arm}}\f$ are the position and rotation of
 * the C-arm that have been specified by setLocation().
 */
Vector3x1 CarmGantry::detectorPositionCA() const
{
    const Vector3x1 detectorOffset = _location.rotation * Vector3x1(0.0, 0.0, -_cArmSpan);

    return _location.position - detectorOffset;
}

/*!
 * Returns the current detector rotation (i.e. transformation matrix from world to CT coordinates).
 *
 * In this configuration, this is the same as the transposed source rotation matrix, i.e.:
 * location().rotation.transposed().
 *
 * \f$
 * R_{\textrm{det}}^{\textrm{nominal}}=R_{\textrm{C-arm}}^{T},
 * \f$
 *
 * where \f$R_{\textrm{C-arm}}\f$ denotes the rotation of the C-arm that has been specified by
 * setLocation().
 */
Matrix3x3 CarmGantry::detectorRotationCA() const
{
    return _location.rotation.transposed();
}

/*!
 * Convenience method. Returns the current detector location (i.e. position and location).
 *
 * Same as mat::Location(detectorPositionCA(), detectorRotationCA()).
 */
mat::Location CarmGantry::detectorLocationCA() const
{
    return { detectorPositionCA(), detectorRotationCA() };
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base classes (GenericGantry and GenericComponent), the
 * info string contains the following details:
 * \li C-arm span,
 * \li (nominal) Source location.
 */
QString CarmGantry::info() const
{
    QString ret(AbstractGantry::info());

    QString position = "(" +
            QString::number(_location.position.get<0>()) + " mm, " +
            QString::number(_location.position.get<1>()) + " mm, " +
            QString::number(_location.position.get<2>()) + " mm)";

    ret +=
        typeInfoString(typeid(this)) +
        "\tC-arm span: " + QString::number(_cArmSpan) + " mm\n" +
        "\tSource position: " + position + "\n" +
        "\tSource rotation:\n" + QString::fromStdString(_location.rotation.info("\t"));

    ret += (this->type() == CarmGantry::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Returns the default name for the component: "C-arm gantry".
 */
QString CarmGantry::defaultName()
{
    static const QString defName(QStringLiteral("C-arm gantry"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

// Use SerializationInterface::fromVariant() documentation.
void CarmGantry::fromVariant(const QVariant& variant)
{
    AbstractGantry::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _cArmSpan = varMap.value("c-arm span").toDouble();
    _location.fromVariant(varMap.value("location"));
}

// Use SerializationInterface::toVariant() documentation.
QVariant CarmGantry::toVariant() const
{
    QVariantMap ret = AbstractGantry::toVariant().toMap();

    ret.insert("c-arm span", _cArmSpan);
    ret.insert("location", location().toVariant());

    return ret;
}

// use documentation of GenericComponent::clone()
SystemComponent* CarmGantry::clone() const { return new CarmGantry(*this); }

/*!
 * Returns the current location of the gantry, i.e. the position (in world coordinates) of the
 * source and its rotation.
 */
const mat::Location& CarmGantry::location() const { return _location; }

/*!
 * Returns the span of the C-arm, i.e. the distance between source and detector (in mm).
 */
double CarmGantry::cArmSpan() const { return _cArmSpan; }

/*!
 * Sets the location of the gantry to \a location. This contains the position (in world coordinates)
 * of the source and its rotation.
 */
void CarmGantry::setLocation(const mat::Location& location) { _location = location; }

/*!
 * Sets the span of the C-arm, i.e. the distance between source and detector (in mm), to \a span.
 */
void CarmGantry::setCarmSpan(double span) { _cArmSpan = span; }

} // namespace CTL
