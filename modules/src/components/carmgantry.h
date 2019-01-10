#ifndef CARMGANTRY_H
#define CARMGANTRY_H

#include "abstractgantry.h"

namespace CTL {
/*!
 * \class CarmGantry
 *
 * \brief Specialized sub-class of AbstractGantry to represent systems using a C-arm mounting
 * for source and detector.
 *
 * The CarmGantry class provides a convenient means to describe a system geometry in which detector
 * and source have a fixed mechanical connection, as it is typical in C-arm mounted CT systems. To
 * fully describe the configuration, the location of the source (i.e. position and rotation, see
 * also AbstractGantry) needs to be given as well as the distance between source and detector.
 *
 * It is assumed that source and detector are aligned on the optical axis, i.e. a ray from the
 * source that goes through the isocenter hits the center of the detector. (Use the displacement
 * concept if you want to consider misalignments. See AbstractGantry documentation for details on
 * displacement).
 */
class CarmGantry : public AbstractGantry
{
    ADD_TO_COMPONENT_ENUM(210)

    // implementation of abstract interface
    protected: mat::Location nominalDetectorLocation() const override;
    protected: mat::Location nominalSourceLocation() const override;

public:
    CarmGantry(double cArmSpan = 1000.0, const QString& name = defaultName());
    CarmGantry(const QJsonObject& json);

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override;    // JSON
    void write(QJsonObject& json) const override;   // JSON

    // getter methods
    const mat::Location& location() const;
    double cArmSpan() const;

    // setter methods
    void setLocation(const mat::Location& location);
    void setCarmSpan(double span);

    // static methods
    static QString defaultName();

protected:
    double _cArmSpan;
    mat::Location _location;

private:
    // helper methods
    mat::Location detectorLocationCA() const;
    Vector3x1 detectorPositionCA() const;
    Matrix3x3 detectorRotationCA() const;
};

// use documentation of GenericComponent::clone()
inline SystemComponent* CarmGantry::clone() const { return new CarmGantry(*this); }

/*!
 * Returns the nominal detector location. This ignores any (optional) detector displacement.
 *
 * Overrides the base class method and computes the detector location based on the C-arm
 * parametrization (i.e. source location and C-arm span).
 */
inline mat::Location CarmGantry::nominalDetectorLocation() const { return detectorLocationCA(); }

/*!
 * Returns the nominal source location. This ignores any (optional) source displacement.
 *
 * Overrides the base class method and returns the source location like specified as part of the
 * C-arm parametrization (i.e. source location and C-arm span).
 *
 * \sa location().
 */
inline mat::Location CarmGantry::nominalSourceLocation() const { return _location; }

/*!
 * Returns the current location of the gantry, i.e. the position (in world coordinates) of the
 * source and its rotation.
 */
inline const mat::Location& CarmGantry::location() const { return _location; }

/*!
 * Returns the span of the C-arm, i.e. the distance between source and detector (in mm).
 */
inline double CarmGantry::cArmSpan() const { return _cArmSpan; }

/*!
 * Sets the location of the gantry to \a location. This contains the position (in world coordinates)
 * of the source and its rotation.
 */
inline void CarmGantry::setLocation(const mat::Location& location) { _location = location; }

/*!
 * Sets the span of the C-arm, i.e. the distance between source and detector (in mm), to \a span.
 */
inline void CarmGantry::setCarmSpan(double span) { _cArmSpan = span; }

} // namespace CTL

/*! \file */

#endif // CARMGANTRY_H
