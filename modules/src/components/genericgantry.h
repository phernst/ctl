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
    CTL_TYPE_ID(201)

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

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

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
    mat::Location _detectorLocation; //!< Location of the detector component.
    mat::Location _sourceLocation; //!< Location of the source component.
};

} // namespace CTL

/*! \file */


#endif // GENERICGANTRY_H
