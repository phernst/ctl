#ifndef PREPARATIONPROTOCOLS_H
#define PREPARATIONPROTOCOLS_H

// trajectories as special protocol are separated into "trajectories.h"
#include "trajectories.h"

namespace CTL {
namespace protocols {

class FlyingFocalSpot : public AbstractPreparationProtocol
{
public:
    FlyingFocalSpot(std::vector<Vector3x1> positions);

    std::vector<std::shared_ptr<AbstractPrepareStep>> prepareSteps(uint viewNb, const AcquisitionSetup& setup) const override;
    bool isApplicableTo(const AcquisitionSetup& setup) const override;

private:
    std::vector<Vector3x1> _positions;
};

class TubeCurrentModulation : public AbstractPreparationProtocol
{
public:
    TubeCurrentModulation(std::vector<double> currents);

    std::vector<std::shared_ptr<AbstractPrepareStep>> prepareSteps(uint viewNb, const AcquisitionSetup& setup) const override;
    bool isApplicableTo(const AcquisitionSetup& setup) const override;

private:
    std::vector<double> _currents;
};

} // namespace protocols
} // namespace CTL

#endif // PREPARATIONPROTOCOLS_H
