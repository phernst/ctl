#ifndef ABSTRACTGANTRY_H
#define ABSTRACTGANTRY_H

#include "mat/matrix_utils.h"
#include "systemcomponent.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {

/*!
 * \class AbstractGantry
 *
 * \brief Base class for gantry components.
 *
 * This class manages the positioning of a source and a detector component. This refers to the
 * components' positions in world coordinate system (WCS) and the transformation matrices that
 * describe the transformation between the component's intrinsic system (see below for details)
 * and the WCS.
 *
 * The location (i.e. position and rotation) of the components is interpreted as follows:
 *
 * Source: the position refers to the point in space (in WCS) where the radiation is originating
 * from (e.g. the focal spot of an X-ray tube). The rotation matrix describes the transformation
 * from the source coordinate system to the WCS. Here, the source coordinate system means an
 * unaltered CT coordinate system (right-handed), where the source is located at the origin (0,0,0),
 * the detector is in the x-y-plane and the z-axis points towards the detector.
 *
 * Detector: the position refers to the mechanical center of the detector system in world
 * coordinates. For example: In case of a flat panel detector in an unaltered CT system, this would
 * be the position of the principal point expressed in world coordinates. The rotation matrix
 * describes the transformation from WCS to CT coordinates of the entire detector system.
 *
 * The AbstractGantry class provides the option to specify so-called displacements for the source
 * and detector (both individually). This displacement is considered to represent deviations from
 * the actual positioning of the components. It can be used e.g. to simulate errors in the mounting
 * of the compontents and/or influences from various other sources of disturbance (e.g. inaccuracies
 * in position reproducability, deformations due to mechanical stress, movement etc.) The
 * displacement consists of a position term (to describe translational dislocation) and a rotation
 * matrix (to explain orientation deviation). The effect of displacement is automatically considered
 * when querying the location of source or detector via sourceLocation() and detectorLocation() (or
 * their convenience alternatives, respectively).
 *
 * Sub-classes of AbstractGantry can be created to use customized parametrizations for the location
 * of source and detector. To do so, override the methods nominalDetectorLocation() and
 * nominalSourceLocation() in the new class, such that these methods extract the location of both
 * components (i.e. source and detector) based on your chosen parametrization.
 *
 * When creating a sub-class of AbstractGantry, make sure to register the new component in the
 * enumeration using the #CTL_TYPE_ID(newIndex) macro. It is required to specify a value
 * for \a newIndex that is not already in use. This can be easily achieved by use of values starting
 * from GenericComponent::UserType, as these are reserved for user-defined types.
 *
 * To provide full compatibility within existing functionality, it is recommended to reimplement the
 * read() and write() method, such that these cover newly introduced information of the sub-class.
 * The new class should then also be added to switch-case list inside the implementation of
 * parseComponentFromJson(const QJsonObject&) found in the header file "components/jsonparser.h".
 */
class AbstractGantry : public SystemComponent
{
    CTL_TYPE_ID(200)
    DECLARE_ELEMENTAL_TYPE

    // abstract interface
    protected:virtual mat::Location nominalDetectorLocation() const = 0;
    protected:virtual mat::Location nominalSourceLocation() const = 0;

public:
    AbstractGantry(const QString& name);

    // virtual methods
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter methods
    mat::Location sourceLocation() const;
    mat::Location detectorLocation() const;
    const mat::Location& detectorDisplacement() const;
    const mat::Location& sourceDisplacement() const;

    // setter methods
    void setDetectorDisplacement(const mat::Location& displacement);
    void setSourceDisplacement(const mat::Location& displacement);

    // convenience getter
    Vector3x1 sourcePosition() const;
    Matrix3x3 sourceRotation() const;
    Vector3x1 detectorPosition() const;
    Matrix3x3 detectorRotation() const;

    // convenience setter
    void setDetectorDisplacementAngles(double rollAngle, double tiltAngle, double twistAngle);
    void setDetectorDisplacementPosition(const Vector3x1& position);
    void setDetectorDisplacementPosition(double x, double y, double z);
    void setSourceDisplacementPosition(const Vector3x1& position);
    void setSourceDisplacementPosition(double x, double y, double z);

protected:
    AbstractGantry() = default;

private:
    mat::Location _detectorDisplacement;
    mat::Location _sourceDisplacement;
};

/*!
 * Constructs an AbstractGantry object named \a name.
 */
inline AbstractGantry::AbstractGantry(const QString& name)
    : SystemComponent(name)
{
}

/*!
 * Returns the final source location. This considers (optional) effects of a source displacement.
 *
 * The location contains the position in world coordinates as well as the rotation matrix that
 * holds the transformation from an unaltered source coordinate system to the WCS.
 */
inline mat::Location AbstractGantry::sourceLocation() const
{
    return { sourcePosition(), sourceRotation() };
}

/*!
 * Convenience method. Returns the final source position in world coordinates.
 *
 * Same as: sourceLocation().position.
 */
inline Vector3x1 AbstractGantry::sourcePosition() const
{
    return nominalSourceLocation().position + sourceRotation() * _sourceDisplacement.position;
}

/*!
 * Convenience method. Returns the total transformation matrix from an unaltered source coordinate
 * system to the WCS.
 *
 * Same as sourceLocation().rotation.
 */
inline Matrix3x3 AbstractGantry::sourceRotation() const
{
    return nominalSourceLocation().rotation * _sourceDisplacement.rotation;
}

/*!
 * Returns the final detector location. This considers (optional) effects of a detector
 * displacement.
 *
 * The location contains the position in world coordinates as well as the rotation matrix that
 * holds the transformation from world coordinates to the CT coordinate system of the detector as
 * a whole.
 */
inline mat::Location AbstractGantry::detectorLocation() const
{
    return { detectorPosition(), detectorRotation() };
}

/*!
 * Convenience method. Returns the final position of the detector center in world coordinates.
 *
 * Same as: detectorLocation().position.
 */
inline Vector3x1 AbstractGantry::detectorPosition() const
{
    return nominalDetectorLocation().position
        + detectorRotation().transposed() * _detectorDisplacement.position;
}

/*!
 * Convenience method. Returns the total transformation matrix from world coordinates to the CT
 * coordinate system of the detector as a whole.
 *
 * Same as: detectorLocation().rotation.
 */
inline Matrix3x3 AbstractGantry::detectorRotation() const
{
    return _detectorDisplacement.rotation * nominalDetectorLocation().rotation;
}

/*!
 *  Returns the specified displacement of the detector.
 */
inline const mat::Location& AbstractGantry::detectorDisplacement() const
{
    return _detectorDisplacement;
}

/*!
 *  Returns the specified displacement of the source.
 */
inline const mat::Location& AbstractGantry::sourceDisplacement() const
{
    return _sourceDisplacement;
}

/*!
 *  Sets the displacement of the detector to \a displacement.
 */
inline void AbstractGantry::setDetectorDisplacement(const mat::Location& displacement)
{
    _detectorDisplacement = displacement;
}

/*!
 *  Sets the displacement of the source to \a displacement.
 */
inline void AbstractGantry::setSourceDisplacement(const mat::Location& displacement)
{
    _sourceDisplacement = displacement;
}

/*!
 *  Convenience setter. Sets the position component of the detector displacement to \a position.
 *
 * \sa setDetectorDisplacement()
 */
inline void AbstractGantry::setDetectorDisplacementPosition(const Vector3x1& position)
{
    _detectorDisplacement.position = position;
}

/*!
 *  Convenience setter. Sets the position component of the detector displacement to (\a x, \a y, \a
 * z).
 *
 * \sa setDetectorDisplacement()
 */
inline void AbstractGantry::setDetectorDisplacementPosition(double x, double y, double z)
{
    _detectorDisplacement.position = Vector3x1(x, y, z);
}

/*!
 *  Convenience setter. Sets the position component of the source displacement to \a position.
 *
 * \sa setSourceDisplacement()
 */
inline void AbstractGantry::setSourceDisplacementPosition(const Vector3x1& position)
{
    _sourceDisplacement.position = position;
}

/*!
 *  Convenience setter. Sets the position component of the source displacement to (\a x, \a y, \a
 * z).
 *
 * \sa setSourceDisplacement()
 */
inline void AbstractGantry::setSourceDisplacementPosition(double x, double y, double z)
{
    _sourceDisplacement.position = Vector3x1(x, y, z);
}

/*!
 * Convenience setter. Sets the rotation component of the detector displacement regarding
 * three rotations specified by \a rollAngle,\a tiltAngle and \a twistAngle. In CT coordinates,
 * these rotations refer to:
 *
 * \li Roll:  rotation around the y-axis
 * \li Tilt:  rotation around the x-axis
 * \li Twist: rotation around the z-axis
 *
 * \sa setDetectorDisplacement()
 */
inline void
AbstractGantry::setDetectorDisplacementAngles(double rollAngle, double tiltAngle, double twistAngle)
{
    _detectorDisplacement.rotation = mat::rotationMatrix(twistAngle, Qt::ZAxis)
        * mat::rotationMatrix(tiltAngle, Qt::XAxis) * mat::rotationMatrix(rollAngle, Qt::YAxis);
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class (GenericComponent), the info string contains
 * the following details:
 * \li Source Displacement
 * \li Detector Displacement.
 */
inline QString AbstractGantry::info() const
{
    QString ret(SystemComponent::info());

    // clang-format off
    QString srcDisplString = "(" + QString::number(_sourceDisplacement.position(0,0)) + " mm, " +
                                   QString::number(_sourceDisplacement.position(1,0)) + " mm, " +
                                   QString::number(_sourceDisplacement.position(2,0)) + " mm)\n";
    QString detDisplString = "(" + QString::number(_detectorDisplacement.position(0,0)) + " mm, " +
                                   QString::number(_detectorDisplacement.position(1,0)) + " mm, " +
                                   QString::number(_detectorDisplacement.position(2,0)) + " mm)\n";
    QString srcRotString = QString::fromStdString(_sourceDisplacement.rotation.info("\t"));
    QString detRotString = QString::fromStdString(_detectorDisplacement.rotation.info("\t"));

    ret +=
        typeInfoString(typeid(this)) +
        "\tSource Displacement: "    + srcDisplString +
        "\t-Rotation:\n"              + srcRotString +
        "\tDetector Displacement: "  + detDisplString +
        "\t-Rotation:\n"             + detRotString;
    // clang-format on

    ret += (this->type() == AbstractGantry::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
inline void AbstractGantry::fromVariant(const QVariant& variant)
{
    SystemComponent::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _detectorDisplacement.fromVariant(varMap.value("detector displacement"));
    _sourceDisplacement.fromVariant(varMap.value("source displacement"));
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
inline QVariant AbstractGantry::toVariant() const
{
    QVariantMap ret = SystemComponent::toVariant().toMap();

    ret.insert("detector displacement", _detectorDisplacement.toVariant());
    ret.insert("source displacement", _sourceDisplacement.toVariant());

    return ret;
}

} // namespace CTL

/*! \file */
///@{
/*!
 * \fn std::unique_ptr<GenericGantry> CTL::makeGantry(ConstructorArguments&&... arguments)
 * \relates GenericGantry
 *
 * Factory method to construct a GenericGantry and wrap the object in a
 * std::unique_ptr<GenericGantry>.
 *
 * This is similar to the more general method GenericComponent::makeComponent() with the difference
 * that it returns a unique pointer to the GenericGantry base type instead of GenericComponent.
 *
 * \sa GenericComponent::makeComponent().
 */
///@}

#endif // ABSTRACTGANTRY_H
