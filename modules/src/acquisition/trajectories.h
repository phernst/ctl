#ifndef CTL_TRAJECTORIES_H
#define CTL_TRAJECTORIES_H

#include "abstractpreparestep.h"
#include "mat/mat.h"

namespace CTL {
namespace protocols {

class HelicalTrajectory : public AbstractPreparationProtocol
{
public:
    HelicalTrajectory(double angleIncrement,
                      double pitchIncrement = 0.0,
                      double startPitch = 0.0,
                      double startAngle = 0.0_deg);

    std::vector<std::shared_ptr<AbstractPrepareStep>> prepareSteps(uint viewNb, const AcquisitionSetup& setup) const override;
    bool isApplicableTo(const AcquisitionSetup& setup) const override;

    void setAngleIncrement(double angleIncrement);
    void setPitchIncrement(double pitchIncrement);
    void setStartPitch(double startPitch);
    void setStartAngle(double startAngle);

private:
    double _angleIncrement = 0.0;
    double _pitchIncrement = 0.0;
    double _startPitch     = 0.0;
    double _startAngle     = 0.0;
};

class WobbleTrajectory : public AbstractPreparationProtocol
{
public:
    WobbleTrajectory(double angleSpan,
                     double sourceToIsocenter,
                     double startAngle  = 0.0_deg,
                     double wobbleAngle = 15.0_deg,
                     double wobbleFreq  = 1.0);

    std::vector<std::shared_ptr<AbstractPrepareStep>> prepareSteps(uint viewNb, const AcquisitionSetup& setup) const override;
    bool isApplicableTo(const AcquisitionSetup& setup) const override;

    void setStartAngle(double startAngle);
    void setWobbleAngle(double wobbleAngle);
    void setWobbleFreq(double wobbleFreq);

private:
    double _angleSpan         = 0.0;
    double _sourceToIsocenter = 0.0;
    double _startAngle        = 0.0;
    double _wobbleAngle       = 0.0;
    double _wobbleFreq        = 0.0;
};

class CirclePlusLineTrajectory : public AbstractPreparationProtocol
{
public:
    CirclePlusLineTrajectory(double angleSpan,
                             double sourceToIsocenter,
                             double lineLength,
                             double fractionOfViewsForLine = 0.5,
                             double startAngle = 0.0_deg);

    std::vector<std::shared_ptr<AbstractPrepareStep>> prepareSteps(uint viewNb, const AcquisitionSetup& setup) const override;
    bool isApplicableTo(const AcquisitionSetup& setup) const override;

private:
    double _angleSpan              = 0.0;
    double _sourceToIsocenter      = 0.0;
    double _lineLength             = 0.0;
    double _fractionOfViewsForLine = 0.5;
    double _startAngle             = 0.0;
};

class ShortScanTrajectory : public AbstractPreparationProtocol
{
public:
    ShortScanTrajectory(double sourceToIsocenter,
                        double startAngle = 0.0_deg,
                        double angleSpan = -1.0_deg);

    std::vector<std::shared_ptr<AbstractPrepareStep>> prepareSteps(uint viewNb, const AcquisitionSetup& setup) const override;
    bool isApplicableTo(const AcquisitionSetup& setup) const override;

private:
    double _sourceToIsocenter = 0.0;
    double _startAngle        = 0.0;
    double _angleSpan         = -1.0;

    double fanAngle(const AcquisitionSetup& setup) const;
};

class AxialScanTrajectory : public AbstractPreparationProtocol
{
public:
    AxialScanTrajectory(double startAngle = 0.0_deg);

    std::vector<std::shared_ptr<AbstractPrepareStep>> prepareSteps(uint viewNb, const AcquisitionSetup& setup) const override;
    bool isApplicableTo(const AcquisitionSetup& setup) const override;

    void setStartAngle(double startAngle);

private:
    double _startAngle = 0.0;
};

} // namespace protocols
} // namespace CTL

#endif // CTL_TRAJECTORIES_H
