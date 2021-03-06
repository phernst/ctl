#ifndef CTL_CARMGANTRY_H
#define CTL_CARMGANTRY_H

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
    CTL_TYPE_ID(210)

    // implementation of abstract interface
    protected: mat::Location nominalDetectorLocation() const override;
    protected: mat::Location nominalSourceLocation() const override;

public:
    CarmGantry(double cArmSpan = 1000.0, const QString& name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter methods
    const mat::Location& location() const;
    double cArmSpan() const;

    // setter methods
    void setLocation(const mat::Location& location);
    void setCarmSpan(double span);

    // static methods
    static QString defaultName();

protected:
    double _cArmSpan; //!< Span of the C-arm, i.e. distance between source and detector (in mm).
    mat::Location _location; //!< Location of the source (rotation is oriented towards detector).

private:
    // helper methods
    mat::Location detectorLocationCA() const;
    Vector3x1 detectorPositionCA() const;
    Matrix3x3 detectorRotationCA() const;
};

} // namespace CTL

/*! \file */

#endif // CTL_CARMGANTRY_H
