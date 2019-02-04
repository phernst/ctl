#include <QDebug>

#include "preparationprotocols.h"
#include "preparesteps.h"
#include "acquisitionsetup.h"

namespace CTL {
namespace protocols {

FlyingFocalSpot::FlyingFocalSpot(std::vector<Vector3x1> positions, bool alternating)
    : _positions(std::move(positions))
    , _alternating(alternating)
{
}

FlyingFocalSpot FlyingFocalSpot::twoAlternatingSpots(const Vector3x1 &position1,
                                                     const Vector3x1 &position2)
{
    FlyingFocalSpot ret;
    ret._alternating = true;
    ret._positions.push_back(position1);
    ret._positions.push_back(position2);

    return ret;
}

FlyingFocalSpot FlyingFocalSpot::fourAlternatingSpots(const Vector3x1 &position1,
                                                      const Vector3x1 &position2,
                                                      const Vector3x1 &position3,
                                                      const Vector3x1 &position4)
{
    FlyingFocalSpot ret;
    ret._alternating = true;
    ret._positions.push_back(position1);
    ret._positions.push_back(position2);
    ret._positions.push_back(position3);
    ret._positions.push_back(position4);

    return ret;
}

std::vector<std::shared_ptr<AbstractPrepareStep>>
FlyingFocalSpot::prepareSteps(uint viewNb, const AcquisitionSetup&) const
{
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    const auto& fsPos = _alternating ? _positions[viewNb%_positions.size()]
                                     : _positions[viewNb];

    auto srcPrep = std::make_shared<prepare::SourceParam>();
    srcPrep->setFocalSpotPosition(fsPos);
    ret.push_back(srcPrep);

    qDebug() << "FlyingFocalSpot --- add prepare steps for view: " << viewNb
             << "\n-position: " << QString::fromStdString(fsPos.info());

    return ret;
}

bool FlyingFocalSpot::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::SourceParam tmp;
    // check whether positions are available for all requested views (unnecessary when alternating)
    bool sizeFitCheck = _alternating || (_positions.size() == setup.nbViews());

    return tmp.isApplicableTo(*setup.system())&&
            sizeFitCheck;
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
