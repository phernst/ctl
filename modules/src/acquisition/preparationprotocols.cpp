#include <QDebug>

#include "preparationprotocols.h"
#include "preparesteps.h"
#include "acquisitionsetup.h"

namespace CTL {
namespace protocols {

FlyingFocalSpot::FlyingFocalSpot(std::vector<Vector3x1> positions)
    : _positions(std::move(positions))
{
}

std::vector<std::shared_ptr<AbstractPrepareStep>>
FlyingFocalSpot::prepareSteps(uint viewNb, const AcquisitionSetup&) const
{
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    auto srcPrep = std::make_shared<prepare::SourceParam>();
    srcPrep->setFocalSpotPosition(_positions[viewNb]);

    ret.push_back(srcPrep);

    qDebug() << "FlyingFocalSpot --- add prepare steps for view: " << viewNb
             << "\n-position: " << QString::fromStdString(_positions[viewNb].info());

    return ret;
}

bool FlyingFocalSpot::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::SourceParam tmp;
    return tmp.isApplicableTo(*setup.system())&&
           (_positions.size() == setup.nbViews());
}

TubeCurrentModulation::TubeCurrentModulation(std::vector<double> currents)
    : _currents(std::move(currents))
{
}

std::vector<std::shared_ptr<AbstractPrepareStep>>
TubeCurrentModulation::prepareSteps(uint viewNb, const AcquisitionSetup&) const
{
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    auto srcPrep = std::make_shared<prepare::XrayTubeParam>();
    srcPrep->setEmissionCurrent(_currents[viewNb]);

    ret.push_back(srcPrep);

    qDebug() << "TubeCurrentModulation --- add prepare steps for view: " << viewNb
             << "\n-tube current: " << QString::number(_currents[viewNb]);

    return ret;
}

bool TubeCurrentModulation::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::XrayTubeParam tmp;
    return tmp.isApplicableTo(*setup.system()) &&
           (_currents.size() == setup.nbViews());
}

} // namespace protocols
} // namespace CTL
