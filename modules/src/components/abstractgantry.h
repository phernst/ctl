#ifndef CTL_ABSTRACTGANTRY_H
#define CTL_ABSTRACTGANTRY_H

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
 * To enable de-/serialization of objects of the new sub-class, reimplement the toVariant() and
 * fromVariant() methods. These should take care of all newly introduced information of the
 * sub-class. Additionally, call the macro #DECLARE_SERIALIZABLE_TYPE(YourNewClassName) within the
 * .cpp file of your new class (substitute "YourNewClassName" with the actual class name). Objects
 * of the new class can then be de-/serialized with any of the serializer classes (see also
 * AbstractSerializer).
 */
class AbstractGantry : public SystemComponent
{
    CTL_TYPE_ID(200)
    DECLARE_ELEMENTAL_TYPE

    // abstract interface
    protected:virtual mat::Location nominalDetectorLocation() const = 0;
    protected:virtual mat::Location nominalSourceLocation() const = 0;

public:
    // virtual methods
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter methods
    mat::Location sourceLocation() const;
    mat::Location detectorLocation() const;
    const mat::Location& detectorDisplacement() const;
    const mat::Location& gantryDisplacement() const;
    const mat::Location& sourceDisplacement() const;

    // setter methods
    void setDetectorDisplacement(const mat::Location& displacement);
    void setGantryDisplacement(const mat::Location& displacement);
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
    void setGantryDisplacementPosition(const Vector3x1& position);
    void setGantryDisplacementPosition(double x, double y, double z);
    void setSourceDisplacementPosition(const Vector3x1& position);
    void setSourceDisplacementPosition(double x, double y, double z);

    ~AbstractGantry() override = default;

protected:
    AbstractGantry() = default;
    AbstractGantry(const QString& name);

    AbstractGantry(const AbstractGantry&) = default;
    AbstractGantry(AbstractGantry&&) = default;
    AbstractGantry& operator=(const AbstractGantry&) = default;
    AbstractGantry& operator=(AbstractGantry&&) = default;

private:
    mat::Location _globalGantryDisplacement; //!< Displacement of the whole gantry.
    mat::Location _detectorDisplacement; //!< Displacement of the detector component.
    mat::Location _sourceDisplacement; //!< Displacement of the source component.
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
 *
 * \f$
 * L_{\textrm{src}}^{\textrm{total}}=
 * (t_{\textrm{src}}^{\textrm{total}},R_{\textrm{src}}^{\textrm{total}})
 * \f$
 *
 * \sa sourcePosition(), sourceRotation().
 */
inline mat::Location AbstractGantry::sourceLocation() const
{
    return { sourcePosition(), sourceRotation() };
}

/*!
 * Convenience method. Returns the final source position in world coordinates.
 *
 * \f$
 * t_{\textrm{src}}^{\textrm{total}}=R_{\textrm{gantry}}^{\textrm{displ}}
 * \cdot t_{\textrm{src}}^{\textrm{nominal}}+R_{\textrm{src}}^{\textrm{total}}
 * \cdot t_{\textrm{src}}^{\textrm{displ}}+t_{\textrm{gantry}}^{\textrm{displ}}
 * \f$
 *
 * Same as: sourceLocation().position.
 */
inline Vector3x1 AbstractGantry::sourcePosition() const
{
    return _globalGantryDisplacement.rotation* nominalSourceLocation().position
            + sourceRotation() * _sourceDisplacement.position
            + _globalGantryDisplacement.position;
}

/*!
 * Convenience method. Returns the total transformation matrix from an unaltered source coordinate
 * system to the WCS.
 *
 * \f$
 * R_{\textrm{src}}^{\textrm{total}}=R_{\textrm{gantry}}^{\textrm{displ}}
 * \cdot R_{\textrm{src}}^{\textrm{nominal}}\cdot R_{\textrm{src}}^{\textrm{displ}}
 * \f$
 *
 * Same as sourceLocation().rotation.
 */
inline Matrix3x3 AbstractGantry::sourceRotation() const
{
    return _globalGantryDisplacement.rotation
            * nominalSourceLocation().rotation
            * _sourceDisplacement.rotation;
}

/*!
 * Returns the final detector location. This considers (optional) effects of a detector
 * displacement.
 *
 * The location contains the position in world coordinates as well as the rotation matrix that
 * holds the transformation from world coordinates to the CT coordinate system of the detector as
 * a whole.
 *
 * \f$
 * L_{\textrm{det}}^{\textrm{total}}=
 * (t_{\textrm{det}}^{\textrm{total}},R_{\textrm{det}}^{\textrm{total}})
 * \f$
 *
 * \sa detectorPosition(), detectorRotation().
 */
inline mat::Location AbstractGantry::detectorLocation() const
{
    return { detectorPosition(), detectorRotation() };
}

/*!
 * Convenience method. Returns the final position of the detector center in world coordinates.
 *
 * \f$
 * \begin{align*}
 * t_{\textrm{det}}^{\textrm{total}} & =R_{\textrm{gantry}}^{\textrm{displ}}\cdot
 * \left(t_{\textrm{det}}^{\textrm{nominal}}+\left(R_{\textrm{det}}^{\textrm{displ}}\right)^{T}
 * \cdot R_{\textrm{det}}^{\textrm{nominal}}\cdot t_{\textrm{det}}^{\textrm{displ}}\right)
 * +t_{\textrm{gantry}}^{\textrm{displ}}\\& =R_{\textrm{gantry}}^{\textrm{displ}}
 * \cdot t_{\textrm{det}}^{\textrm{nominal}}+\left(R_{\textrm{det}}^{\textrm{total}}\right)^{T}
 * \cdot t_{\textrm{det}}^{\textrm{displ}}+t_{\textrm{gantry}}^{\textrm{displ}}
 * \end{align*}
 * \f$
 *
 * Same as: detectorLocation().position.
 */
inline Vector3x1 AbstractGantry::detectorPosition() const
{
    return _globalGantryDisplacement.rotation * nominalDetectorLocation().position
            + detectorRotation().transposed() * _detectorDisplacement.position
            + _globalGantryDisplacement.position;
}

/*!
 * Convenience method. Returns the total transformation matrix from world coordinates to the CT
 * coordinate system of the detector as a whole.
 *
 * \f$
 * R_{\textrm{WCS-CTS}}:=R_{\textrm{det}}^{\textrm{total}}=\left(R_{\textrm{det}}^{\textrm{displ}}
 * \right)^{T}\cdot R_{\textrm{det}}^{\textrm{nominal}}\cdot\left(R_{\textrm{gantry}}^{\textrm{displ}}
 * \right)^{T}
 * \f$
 *
 * Same as: detectorLocation().rotation.
 */
inline Matrix3x3 AbstractGantry::detectorRotation() const
{
    return _detectorDisplacement.rotation.transposed()
            * nominalDetectorLocation().rotation
            * _globalGantryDisplacement.rotation.transposed();
}

/*!
 *  Returns the specified displacement of the detector.
 *
 * \f$
 * L_{\textrm{det}}^{\textrm{displ}}=
 * (t_{\textrm{det}}^{\textrm{displ}},R_{\textrm{det}}^{\textrm{displ}})
 * \f$
 */
inline const mat::Location& AbstractGantry::detectorDisplacement() const
{
    return _detectorDisplacement;
}

/*!
 *  Returns the specified displacement of the whole gantry.
 *
 * \f$
 * L_{\textrm{gantry}}^{\textrm{displ}}=
 * (t_{\textrm{gantry}}^{\textrm{displ}},R_{\textrm{gantry}}^{\textrm{displ}})
 * \f$
 */
inline const mat::Location& AbstractGantry::gantryDisplacement() const
{
    return _globalGantryDisplacement;
}

/*!
 *  Returns the specified displacement of the source.
 *
 * \f$
 * L_{\textrm{src}}^{\textrm{displ}}=
 * (t_{\textrm{src}}^{\textrm{displ}},R_{\textrm{src}}^{\textrm{displ}})
 * \f$
 */
inline const mat::Location& AbstractGantry::sourceDisplacement() const
{
    return _sourceDisplacement;
}

/*!
 *  Sets the displacement of the detector to \a displacement.
 *
 * The detector displacement is defined in the CT coordinate system.
 */
inline void AbstractGantry::setDetectorDisplacement(const mat::Location& displacement)
{
    _detectorDisplacement = displacement;
}

/*!
 *  Sets the displacement of the whole gantry to \a displacement.
 *
 * The gantry displacement is defined in the world coordinate system.
 */
inline void AbstractGantry::setGantryDisplacement(const mat::Location& displacement)
{
    _globalGantryDisplacement = displacement;
}

/*!
 *  Sets the displacement of the source to \a displacement.
 *
 * The source displacement is defined in the world coordinate system.
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

// Use SerializationInterface::fromVariant() documentation.
inline void AbstractGantry::fromVariant(const QVariant& variant)
{
    SystemComponent::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _detectorDisplacement.fromVariant(varMap.value("detector displacement"));
    _globalGantryDisplacement.fromVariant(varMap.value("gantry displacement"));
    _sourceDisplacement.fromVariant(varMap.value("source displacement"));
}

// Use SerializationInterface::toVariant() documentation.
inline QVariant AbstractGantry::toVariant() const
{
    QVariantMap ret = SystemComponent::toVariant().toMap();

    ret.insert("detector displacement", _detectorDisplacement.toVariant());
    ret.insert("gantry displacement", _globalGantryDisplacement.toVariant());
    ret.insert("source displacement", _sourceDisplacement.toVariant());

    return ret;
}

/*!
 * \fn virtual mat::Location AbstractGantry::nominalDetectorLocation() const = 0
 *
 * Returns the nominal location of the detector. Nominal means the positioning without consideration
 * of the displacement. This is the expected (or ideal) location as it would occur in a perfect
 * system.
 *
 * Implement this method in derived classes such that it extracts the location of the detector
 * component based on the parametrization used in that particular sub-class.
 */

/*!
 * \fn virtual mat::Location AbstractGantry::nominalSourceLocation() const = 0
 *
 * Returns the nominal location of the source. Nominal means the positioning without consideration
 * of the displacement. This is the expected (or ideal) location as it would occur in a perfect
 * system.
 *
 * Implement this method in derived classes such that it extracts the location of the source
 * component based on the parametrization used in that particular sub-class.
 */

} // namespace CTL

/*! \file */
///@{
///@}

#endif // CTL_ABSTRACTGANTRY_H
