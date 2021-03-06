#ifndef CTL_PREPARESTEPS_H
#define CTL_PREPARESTEPS_H

#include "abstractpreparestep.h"
#include "simplectsystem.h"
#include "mat/matrix_utils.h"
#include "processing/coordinates.h"

#include <QSizeF>

namespace CTL {
namespace prepare {

// ### ###  ### ###
// ### GANTRIES ###
// ### ###  ### ###

class TubularGantryParam : public AbstractPrepareStep
{
    CTL_TYPE_ID(220)

public:
    void setRotationAngle(double rotation) { _newRotationAngle = {true, rotation}; }
    void setPitchPosition(double pitch)    { _newPitchPosition = {true, pitch}; }
    void setTiltAngle(double tilt)         { _newTiltAngle     = {true, tilt}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    QPair<bool,double> _newRotationAngle = {false, 0.0};
    QPair<bool,double> _newPitchPosition = {false, 0.0};
    QPair<bool,double> _newTiltAngle     = {false, 0.0};
};

class CarmGantryParam : public AbstractPrepareStep
{
    CTL_TYPE_ID(210)

public:
    void setLocation(const mat::Location& location) { _newLocation = {true, location}; }
    void setCarmSpan(double span)                   { _newCarmSpan = {true, span}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    QPair<bool,mat::Location> _newLocation = {false, mat::Location()};
    QPair<bool,double> _newCarmSpan        = {false, 0.0};
};

class GenericGantryParam : public AbstractPrepareStep
{
    CTL_TYPE_ID(201)

public:
    void setDetectorLocation(const mat::Location& location) { _newDetectorLocation = {true, location}; }
    void setSourceLocation(const mat::Location& location)   { _newSourceLocation   = {true, location}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    QPair<bool,mat::Location> _newDetectorLocation = {false, mat::Location()};
    QPair<bool,mat::Location> _newSourceLocation   = {false, mat::Location()};
};

class GantryDisplacementParam : public AbstractPrepareStep
{
    CTL_TYPE_ID(230)

public:
    void setDetectorDisplacement(const mat::Location& displacement) { _newDetectorDisplacement = {true, displacement}; }
    void setGantryDisplacement(const mat::Location& displacement)   { _newGantryDisplacement   = {true, displacement}; }
    void setSourceDisplacement(const mat::Location& displacement)   { _newSourceDisplacement   = {true, displacement}; }

    void incrementDetectorDisplacement(const mat::Location& increment) { _detectorDisplacementIncrement = {true, increment}; }
    void incrementSourceDisplacement(const mat::Location& increment)   { _sourceDisplacementIncrement   = {true, increment}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    QPair<bool,mat::Location> _newDetectorDisplacement = {false, mat::Location()};
    QPair<bool,mat::Location> _newGantryDisplacement   = {false, mat::Location()};
    QPair<bool,mat::Location> _newSourceDisplacement   = {false, mat::Location()};
    QPair<bool,mat::Location> _detectorDisplacementIncrement = {false, mat::Location()};
    QPair<bool,mat::Location> _sourceDisplacementIncrement   = {false, mat::Location()};
};

// ### ### ### ###
// ### SOURCES ###
// ### ### ### ###

class SourceParam : public AbstractPrepareStep
{
    CTL_TYPE_ID(300)

public:
    void setEnergyRangeRestriction(const Range<float>& range) { _energyRangeRestr = {true, range}; }
    void setFluxModifier(double modifier)                     { _newFluxModifier = {true, modifier}; }
    void setFocalSpotSize(const QSizeF &size)                 { _newFocalSpotSize = {true, size}; }
    void setFocalSpotPosition(const Vector3x1 &position)      { _newSpotPosition = {true, position}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

protected:
    QPair<bool,double> _newFluxModifier    = {false, 0.0};
    QPair<bool,QSizeF> _newFocalSpotSize   = {false, QSizeF(0.0, 0.0)};
    QPair<bool,Vector3x1> _newSpotPosition = {false, Vector3x1(0.0)};
    QPair<bool,Range<float>> _energyRangeRestr = {false, { 0.0f, 0.0f } };
};

class XrayLaserParam : public SourceParam
{
    CTL_TYPE_ID(310)

public:
    void setPhotonEnergy(double energy) { _newPhotonEnergy = {true, energy}; }
    void setPower(double power)         { _newPower = {true, power}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    QPair<bool,double> _newPhotonEnergy = {false, 0.0};
    QPair<bool,double> _newPower        = {false, 0.0};
};

class XrayTubeParam : public SourceParam
{
    CTL_TYPE_ID(320)

public:
    void setTubeVoltage(double voltage)     { _newTubeVoltage = {true, voltage}; }
    void setEmissionCurrent(double current) { _newEmissionCurrent = {true, current}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    QPair<bool,double> _newTubeVoltage     = {false, 0.0};
    QPair<bool,double> _newEmissionCurrent = {false, 0.0};
};


// ### ###  ### ###
// ### DETECTOR ###
// ### ###  ### ###

class GenericDetectorParam : public AbstractPrepareStep
{
    CTL_TYPE_ID(101)

public:
    void setModuleLocations(QVector<mat::Location> moduleLocations) { _newModuleLocations = {true, moduleLocations}; }
    void setPixelSize(const QSizeF& size) { _newPixelSize = {true, size}; }
    void setSkewCoefficient(double skewCoefficient) { _newSkewCoefficient = {true, skewCoefficient}; }

    // AbstractPrepareStep interface
    void prepare(SimpleCTSystem& system) const override;
    bool isApplicableTo(const CTSystem &system) const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    QPair<bool,QVector<mat::Location>> _newModuleLocations = {false, QVector<mat::Location>()};
    QPair<bool,QSizeF> _newPixelSize                       = {false, QSizeF()};
    QPair<bool,double> _newSkewCoefficient                 = {false, 0.0};
};

} // namespace prepare
} // namespace CTL

#endif // CTL_PREPARESTEPS_H
