#include "genericgantry.h"

namespace CTL {

DECLARE_JSON_COMPATIBLE_TYPE(GenericGantry)

/*!
 * Constructs a GenericGantry object based on the information specified in the QJsonObject \a json.
 */
GenericGantry::GenericGantry(const QJsonObject& json)
    : AbstractGantry(defaultName())
{
    GenericGantry::read(json);
}

/*!
 * Constructs a GenericGantry object named \a name.
 */
GenericGantry::GenericGantry(const QString &name)
    : AbstractGantry(name)
{
}

/*!
 * Constructs a GenericGantry with the source placed at \a sourceLocation (note that this includes
 * its position and rotation) and the detector mounted at \a detectorLocation.
 *
 * For details on all coordinate systems: see detailed class description.
 */
GenericGantry::GenericGantry(const mat::Location& sourceLocation,
                             const mat::Location& detectorLocation,
                             const QString& name)
    : AbstractGantry(name)
    , _detectorLocation(detectorLocation)
    , _sourceLocation(sourceLocation)
{
}

/*!
 * Constructs a GenericGantry with a source at \a sourcePosition (orientation given by \a
 * sourceRotation) and the detector placed at \a detectorPosition (orientation given by \a
 * detectorRotation).
 *
 * For details on all coordinate systems: see detailed class description.
 */
GenericGantry::GenericGantry(const Vector3x1& sourcePosition,
                             const Matrix3x3& sourceRotation,
                             const Vector3x1& detectorPosition,
                             const Matrix3x3& detectorRotation,
                             const QString& name)
    : AbstractGantry(name)
    , _detectorLocation({ detectorPosition, detectorRotation })
    , _sourceLocation({ sourcePosition, sourceRotation })
{
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class (GenericComponent), the info string contains
 * the following details:
 * \li Source position
 * \li Detector position
 * \li Source rotation
 * \li Detector rotation.
 */
QString GenericGantry::info() const
{
    QString ret(AbstractGantry::info());

    const auto srcLoc = sourceLocation();
    const auto detLoc = detectorLocation();

    // clang-format off
    QString srcPosString = "(" + QString::number(srcLoc.position(0,0)) + " mm, " +
                                 QString::number(srcLoc.position(1,0)) + " mm, " +
                                 QString::number(srcLoc.position(2,0)) + " mm)\n";
    QString detPosString = "(" + QString::number(detLoc.position(0,0)) + " mm, " +
                                 QString::number(detLoc.position(1,0)) + " mm, " +
                                 QString::number(detLoc.position(2,0)) + " mm)\n";
    QString srcRotString = QString::fromStdString(srcLoc.rotation.info("\t"));
    QString detRotString = QString::fromStdString(detLoc.rotation.info("\t"));

    ret +=
        typeInfoString(typeid(this)) +
        "\tSource position: "    + srcPosString +
        "\tDetector position: "  + detPosString +
        "\tSource rotation:\n"   + srcRotString +
        "\tDetector rotation:\n" + detRotString;
    // clang-format on

    ret += (this->type() == GenericGantry::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Returns the default name for the component: "Generic gantry".
 */
QString GenericGantry::defaultName()
{
    static const QString defName(QStringLiteral("Generic gantry"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

void GenericGantry::read(const QJsonObject &json)
{
    AbstractGantry::read(json);

    _detectorLocation.fromVariant(json.value("detector location").toVariant());
    _sourceLocation.fromVariant(json.value("source location").toVariant());
}

void GenericGantry::write(QJsonObject &json) const
{
    AbstractGantry::write(json);

    json.insert("detector location", QJsonValue::fromVariant(_detectorLocation.toVariant()));
    json.insert("source location", QJsonValue::fromVariant(_sourceLocation.toVariant()));
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void GenericGantry::fromVariant(const QVariant& variant)
{
    AbstractGantry::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _detectorLocation.fromVariant(varMap.value("detector location"));
    _sourceLocation.fromVariant(varMap.value("source location"));
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant GenericGantry::toVariant() const
{
    QVariantMap ret = AbstractGantry::toVariant().toMap();

    ret.insert("detector location", _detectorLocation.toVariant());
    ret.insert("source location", _sourceLocation.toVariant());

    return ret;
}

// use documentation of GenericComponent::clone()
SystemComponent* GenericGantry::clone() const { return new GenericGantry(*this); }

/*!
 * Returns the nominal detector location. This ignores any (optional) detector displacement.
 *
 * Override this method when sub-classing GenericGantry allows for customization of the
 * parametrization of the components' locations.
 *
 * \sa detectorLocation().
 */
mat::Location GenericGantry::nominalDetectorLocation() const { return _detectorLocation; }

/*!
 * Returns the nominal source location. This ignores any (optional) source displacement.
 *
 * Override this method when sub-classing GenericGantry allows for customization of the
 * parametrization of the components' locations.
 *
 * \sa sourceLocation().
 */
mat::Location GenericGantry::nominalSourceLocation() const { return _sourceLocation; }

/*!
 *  Sets the location (i.e. position and rotation) of the detector to \a location. Must not contain
 * effects considered by the displacement.
 */
void GenericGantry::setDetectorLocation(const mat::Location& location)
{
    _detectorLocation = location;
}

/*!
 *  Sets the location (i.e. position and rotation) of the source to \a location. Must not contain
 * effects considered by the displacement.
 */
void GenericGantry::setSourceLocation(const mat::Location& location) { _sourceLocation = location; }

/*!
 * Convenience setter. Sets the position of the detector to \a detectorPosition.
 *
 * \sa setDetectorLocation()
 */
void GenericGantry::setDetectorPosition(const Vector3x1& detectorPosition)
{
    _detectorLocation.position = detectorPosition;
}

/*!
 * Convenience setter. Sets the rotation of the detector to \a detectorRotation.
 *
 * The rotation matrix describes the transformation from WCS to CTS.
 *
 * \sa setDetectorLocation()
 */
void GenericGantry::setDetectorRotation(const Matrix3x3& detectorRotation)
{
    _detectorLocation.rotation = detectorRotation;
}

/*!
 * Convenience setter. Sets the position of the source to \a sourcePosition.
 *
 * \sa setSourceLocation()
 */
void GenericGantry::setSourcePosition(const Vector3x1& sourcePosition)
{
    _sourceLocation.position = sourcePosition;
}

/*!
 * Convenience setter. Sets the rotation of the source to \a sourceRotation.
 *
 * The rotation matrix describes the transformation from the intrinsic source coordinate system to
 * the WCS.
 *
 * \sa setSourceLocation()
 */
void GenericGantry::setSourceRotation(const Matrix3x3& sourceRotation)
{
    _sourceLocation.rotation = sourceRotation;
}

} // namespace CTL
