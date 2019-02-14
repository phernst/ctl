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
    CTL_TYPE_ID(220)

    // implementation of abstract interface
    protected: mat::Location nominalDetectorLocation() const override;
    protected: mat::Location nominalSourceLocation() const override;

public:
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
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // setter methods
    void setPitchPosition(double position);
    void setRotationAngle(double angle);
    void setTiltAngle(double angle);

    // static methods
    static QString defaultName();

protected:
    double _sourceToDetectorDistance;  //!< Distance between source and detector (in mm).
    double _sourceToIsoCenterDistance; //!< Distance between source and iso center (in mm).
    double _rotationAngle;             //!< Gantry rotation angle (in rad).
    double _pitchPosition;             //!< Table pitch position (in mm).
    double _tiltAngle;                 //!< Gantry tilt angle (in rad).

private:
    TubularGantry() = default;

    // methods
    Matrix3x3 totalGantryRotation() const;
    mat::Location sourceLocationTG() const;
    mat::Location detectorLocationTG() const;
    Vector3x1 sourcePositionTG() const;
    Matrix3x3 sourceRotationTG() const;
    Vector3x1 detectorPositionTG() const;
    Matrix3x3 detectorRotationTG() const;
};

} // namespace CTL

/*! \file */

#endif // TUBULARGANTRY_H
