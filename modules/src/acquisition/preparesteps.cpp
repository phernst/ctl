#include <QDebug>

#include "components/allcomponents.h"
#include "preparesteps.h"

namespace CTL {
namespace prepare {

// ### ###  ### ###
// ### GANTRIES ###
// ### ###  ### ###

void TubularGantryParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = static_cast<TubularGantry*>(system->gantry());

    qDebug() << "PrepareTubularGantry --- preparing gantry"
             << "\n-rotation\t" << _newRotationAngle
             << "\n-pitch\t\t" << _newPitchPosition
             << "\n-tilt\t\t" << _newTiltAngle;

    if(_newRotationAngle.first)
        gantryPtr->setRotationAngle(_newRotationAngle.second);
    if(_newPitchPosition.first)
        gantryPtr->setPitchPosition(_newPitchPosition.second);
    if(_newTiltAngle.first)
        gantryPtr->setTiltAngle(_newTiltAngle.second);
}

bool TubularGantryParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<TubularGantry*>(system.gantries().first());
}

void CarmGantryParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = static_cast<CarmGantry*>(system->gantry());

    qDebug() << "PrepareCarmGantry --- preparing gantry"
             << "\n-location\t" << _newLocation.first
             << QString::fromStdString(_newLocation.second.position.info())
             << QString::fromStdString(_newLocation.second.rotation.info())
             << "\n-span\t\t" << _newCarmSpan;

    if(_newLocation.first)
        gantryPtr->setLocation(_newLocation.second);
    if(_newCarmSpan.first)
        gantryPtr->setCarmSpan(_newCarmSpan.second);
}

bool CarmGantryParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<CarmGantry*>(system.gantries().first());
}

void GenericGantryParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = static_cast<GenericGantry*>(system->gantry());

    qDebug() << "PrepareGenericGantry --- preparing gantry"
             << "\n-detector location\t" << _newDetectorLocation.first
             << QString::fromStdString(_newDetectorLocation.second.position.info())
             << QString::fromStdString(_newDetectorLocation.second.rotation.info())
             << "\n-source location\t" << _newSourceLocation.first
             << QString::fromStdString(_newSourceLocation.second.position.info())
             << QString::fromStdString(_newSourceLocation.second.rotation.info());

    if(_newDetectorLocation.first)
        gantryPtr->setDetectorLocation(_newDetectorLocation.second);
    if(_newSourceLocation.first)
        gantryPtr->setSourceLocation(_newSourceLocation.second);
}

bool GenericGantryParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<GenericGantry*>(system.gantries().first());
}

void GantryDisplacementParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = system->gantry();

    qDebug() << "PrepareGantryDisplacements --- preparing gantry"
             << "\n-detector displacement\t" << _newDetectorDisplacement.first
             << QString::fromStdString(_newDetectorDisplacement.second.position.info())
             << QString::fromStdString(_newDetectorDisplacement.second.rotation.info())
             << "\n-source location\t" << _newSourceDisplacement.first
             << QString::fromStdString(_newSourceDisplacement.second.position.info())
             << QString::fromStdString(_newSourceDisplacement.second.rotation.info());

    if(_newDetectorDisplacement.first)
        gantryPtr->setDetectorDisplacement(_newDetectorDisplacement.second);
    if(_newSourceDisplacement.first)
        gantryPtr->setSourceDisplacement(_newSourceDisplacement.second);
    if(_detectorDisplacementIncrement.first)
    {
        mat::Location fullDetDispl;
        auto prevDetDispl = gantryPtr->detectorDisplacement();
        fullDetDispl.position
            = prevDetDispl.position + _detectorDisplacementIncrement.second.position;
        fullDetDispl.rotation
            = _detectorDisplacementIncrement.second.rotation * prevDetDispl.rotation;
        gantryPtr->setDetectorDisplacement(fullDetDispl);
    }
    if(_sourceDisplacementIncrement.first)
    {
        mat::Location fullSrcDispl;
        auto prevSrcDispl = gantryPtr->sourceDisplacement();
        fullSrcDispl.position
            = prevSrcDispl.position + _sourceDisplacementIncrement.second.position;
        fullSrcDispl.rotation
            = prevSrcDispl.rotation * _sourceDisplacementIncrement.second.rotation;
        gantryPtr->setSourceDisplacement(fullSrcDispl);
    }
}

bool GantryDisplacementParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple();
}

// ### ### ### ###
// ### SOURCES ###
// ### ### ### ###

void SourceParam::prepare(SimpleCTsystem* system) const
{
    auto sourcePtr = system->source();

    qDebug() << "PrepareAbstractSource --- preparing source"
             << "\n-flux mod\t" << _newFluxModifier
             << "\n-focal spot size\t" << _newFocalSpotSize
             << "\n-focal spot pos\t" << _newSpotPosition.first
             << QString::fromStdString(_newSpotPosition.second.info());

    if(_newFluxModifier.first)
        sourcePtr->setFluxModifier(_newFluxModifier.second);
    if(_newFocalSpotSize.first)
        sourcePtr->setFocalSpotSize(_newFocalSpotSize.second);
    if(_newSpotPosition.first)
        sourcePtr->setFocalSpotPosition(_newSpotPosition.second);
}

bool SourceParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple();
}

void XrayLaserParam::prepare(SimpleCTsystem* system) const
{
    SourceParam::prepare(system);

    auto sourcePtr = static_cast<XrayLaser*>(system->source());

    qDebug() << "PrepareXrayLaser --- preparing source"
             << "\n-energy\t" << _newPhotonEnergy
             << "\n-power\t" << _newPower;

    if(_newPhotonEnergy.first)
        sourcePtr->setPhotonEnergy(_newPhotonEnergy.second);
    if(_newPower.first)
        sourcePtr->setPower(_newPower.second);
}

bool XrayLaserParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<XrayLaser*>(system.sources().first());
}

void XrayTubeParam::prepare(SimpleCTsystem* system) const
{
    SourceParam::prepare(system);

    auto sourcePtr = static_cast<XrayTube*>(system->source());

    qDebug() << "PrepareXrayTube --- preparing source"
             << "\n-voltage\t" << _newTubeVoltage
             << "\n-emission\t" << _newEmissionCurrent;

    if(_newTubeVoltage.first)
        sourcePtr->setTubeVoltage(_newTubeVoltage.second);
    if(_newEmissionCurrent.first)
        sourcePtr->setEmissionCurrent(_newEmissionCurrent.second);
}

bool XrayTubeParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<XrayTube*>(system.sources().first());
}

void GenericDetectorParam::prepare(SimpleCTsystem* system) const
{
    auto detectorPtr = static_cast<GenericDetector*>(system->detector());

    qDebug() << "PrepareGenericDetector --- preparing detector"
             << "\n-module locations\t" << _newModuleLocations.first
             << _newModuleLocations.second.size() << " modules";

    if(_newModuleLocations.first)
        detectorPtr->setModuleLocations(_newModuleLocations.second);
}

bool GenericDetectorParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<GenericDetector*>(system.detectors().first());
}

} // namespace prepare
} // namespace CTL
