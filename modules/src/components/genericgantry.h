#ifndef GENERICGANTRY_H
#define GENERICGANTRY_H

#include "abstractgantry.h"
#include "mat/matrix_utils.h"

namespace CTL {

/*!
 * \class GenericGantry
 *
 * \brief Generic implementation of a gantry component.
 *
 * This class implements AbstractGantry with a straightforward parametrization of the locations
 * of its detector and source component. In particular, this parametrization consists of the
 * locations (in world coordinates) themselves. These are stored as private members. The abstract
 * interface methods nominalDetectorLocation() and nominalSourceLocation() are implemented such that
 * they simply return their corresponding private members.
 *
 * Use the setter methods setDetectorLocation() or setSourceLocation() to change the locations of
 * source and/or detector, respectively. For convenience, setter methods for isolated changes of the
 * position or rotation component of these locations are provided.
 *
 */
class GenericGantry : public AbstractGantry
{
    ADD_TO_COMPONENT_ENUM(201)

    // implementation of abstract interface
    protected: mat::Location nominalDetectorLocation() const override;
    protected: mat::Location nominalSourceLocation() const override;

public:
    GenericGantry(const QString& name = defaultName());
    GenericGantry(const mat::Location& sourceLocation,
                  const mat::Location& detectorLocation,
                  const QString& name = defaultName());
    GenericGantry(const Vector3x1& sourcePosition,
                  const Matrix3x3& sourceRotation,
                  const Vector3x1& detectorPosition,
                  const Matrix3x3& detectorRotation,
                  const QString& name = defaultName());
    GenericGantry(const QJsonObject& json);


    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // setter methods
    void setDetectorLocation(const mat::Location& location);
    void setSourceLocation(const mat::Location& location);

    // convenience setter
    void setDetectorPosition(const Vector3x1& detectorPosition);
    void setDetectorRotation(const Matrix3x3& detectorRotation);
    void setSourcePosition(const Vector3x1& sourcePosition);
    void setSourceRotation(const Matrix3x3& sourceRotation);

    // static methods
    static QString defaultName();

private:
    mat::Location _detectorLocation;
    mat::Location _sourceLocation;
};

// use documentation of GenericComponent::clone()
inline SystemComponent* GenericGantry::clone() const { return new GenericGantry(*this); }

/*!
 * Returns the nominal detector location. This ignores any (optional) detector displacement.
 *
 * Override this method when sub-classing GenericGantry allows for customization of the
 * parametrization of the components' locations.
 *
 * \sa detectorLocation().
 */
inline mat::Location GenericGantry::nominalDetectorLocation() const { return _detectorLocation; }

/*!
 * Returns the nominal source location. This ignores any (optional) source displacement.
 *
 * Override this method when sub-classing GenericGantry allows for customization of the
 * parametrization of the components' locations.
 *
 * \sa sourceLocation().
 */
inline mat::Location GenericGantry::nominalSourceLocation() const { return _sourceLocation; }

/*!
 *  Sets the location (i.e. position and rotation) of the detector to \a location. Must not contain
 * effects considered by the displacement.
 */
inline void GenericGantry::setDetectorLocation(const mat::Location& location)
{
    _detectorLocation = location;
}

/*!
 *  Sets the location (i.e. position and rotation) of the source to \a location. Must not contain
 * effects considered by the displacement.
 */
inline void GenericGantry::setSourceLocation(const mat::Location& location) { _sourceLocation = location; }

/*!
 * Convenience setter. Sets the position of the detector to \a detectorPosition.
 *
 * \sa setDetectorLocation()
 */
inline void GenericGantry::setDetectorPosition(const Vector3x1& detectorPosition)
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
inline void GenericGantry::setDetectorRotation(const Matrix3x3& detectorRotation)
{
    _detectorLocation.rotation = detectorRotation;
}

/*!
 * Convenience setter. Sets the position of the source to \a sourcePosition.
 *
 * \sa setSourceLocation()
 */
inline void GenericGantry::setSourcePosition(const Vector3x1& sourcePosition)
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
inline void GenericGantry::setSourceRotation(const Matrix3x3& sourceRotation)
{
    _sourceLocation.rotation = sourceRotation;
}

} // namespace CTL

/*! \file */


#endif // GENERICGANTRY_H
