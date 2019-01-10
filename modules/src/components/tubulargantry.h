#ifndef TUBULARGANTRY_H
#define TUBULARGANTRY_H

#include "abstractgantry.h"

namespace CTL {
/*!
 * \class TubularGantry
 *
 * \brief Specialized sub-class of AbstractGantry to represent systems with tube gantries.
 *
 * This gantry sub-class allows for simplified description of tubular gantry systems. Instead
 * of the need to specify source and detector position (and orientation) individually, the
 * configuration of the TubularGantry can be fully described by the following parameters:
 *
 * \li Source-to-detector distance
 * \li Source-to-isocenter distance
 * \li Gantry rotation angle
 * \li Table pitch position
 * \li Gantry tilt angle (optional)
 *
 * ![Coordinate system of a tubular gantry. The position of source f, isocenter o and detector d lay on a straight line.](tubeCS.png)
 * \n
 * ![Definition of gantry rotation angle alpha.](gantryCS.png)
 */
class TubularGantry : public AbstractGantry
{
    ADD_TO_COMPONENT_ENUM(220)

    // implementation of abstract interface
    protected: mat::Location nominalDetectorLocation() const override;
    protected: mat::Location nominalSourceLocation() const override;

public:
    TubularGantry(const QJsonObject& json);
    TubularGantry(double sourceToDetectorDistance,
                  double sourceToIsoCenterDistance,
                  double rotationAngle = 0.0,
                  double pitchPosition = 0.0,
                  double tiltAngle = 0.0,
                  const QString& name = defaultName());
    TubularGantry(double sourceToDetectorDistance,
                  double sourceToIsoCenterDistance,
                  const QString& name);


    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // setter methods
    void setPitchPosition(double position);
    void setRotationAngle(double angle);
    void setTiltAngle(double angle);

    // static methods
    static QString defaultName();

protected:
    double _sourceToDetectorDistance;
    double _sourceToIsoCenterDistance;
    double _rotationAngle;
    double _pitchPosition;
    double _tiltAngle;

private:
    // methods
    Matrix3x3 totalGantryRotation() const;
    mat::Location sourceLocationTG() const;
    mat::Location detectorLocationTG() const;
    Vector3x1 sourcePositionTG() const;
    Matrix3x3 sourceRotationTG() const;
    Vector3x1 detectorPositionTG() const;
    Matrix3x3 detectorRotationTG() const;
};

// use documentation of GenericComponent::clone()
inline SystemComponent* TubularGantry::clone() const { return new TubularGantry(*this); }

/*!
 * Returns the nominal detector location. This ignores any (optional) detector displacement.
 *
 * Overrides the base class method and computes the detector location based on the tubular gantry
 * parametrization (i.e. source-to-detector distance etc.).
 */
inline mat::Location TubularGantry::nominalDetectorLocation() const { return detectorLocationTG(); }

/*!
 * Returns the nominal source location. This ignores any (optional) source displacement.
 *
 * Overrides the base class method and computes the source location based on the tubular gantry
 * parametrization (i.e. source-to-detector distance etc.).
 */
inline mat::Location TubularGantry::nominalSourceLocation() const { return sourceLocationTG(); }

/*!
 * Sets the table pitch to \a position. Position value is expected in millimeters.
 */
inline void TubularGantry::setPitchPosition(double position) { _pitchPosition = position; }

/*!
 * Sets the gantry rotation to \a angle. Rotation angle is expected in radians.
 */
inline void TubularGantry::setRotationAngle(double angle) { _rotationAngle = angle; }

/*!
 * Sets the gantry tilt to \a angle. Tilt angle is expected in radians.
 */
inline void TubularGantry::setTiltAngle(double angle) { _tiltAngle = angle; }

} // namespace CTL

/*! \file */

#endif // TUBULARGANTRY_H
