#include <QDebug>

#include "components/allcomponents.h"
#include "preparesteps.h"

namespace CTL {
namespace prepare {

DECLARE_SERIALIZABLE_TYPE(GenericDetectorParam)
DECLARE_SERIALIZABLE_TYPE(GenericGantryParam)
DECLARE_SERIALIZABLE_TYPE(CarmGantryParam)
DECLARE_SERIALIZABLE_TYPE(TubularGantryParam)
DECLARE_SERIALIZABLE_TYPE(GantryDisplacementParam)
DECLARE_SERIALIZABLE_TYPE(SourceParam)
DECLARE_SERIALIZABLE_TYPE(XrayLaserParam)
DECLARE_SERIALIZABLE_TYPE(XrayTubeParam)

// ### ###  ### ###
// ### GANTRIES ###
// ### ###  ### ###

void TubularGantryParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = static_cast<TubularGantry*>(system->gantry());

    qDebug() << "PrepareTubularGantry --- preparing gantry\n"
             << "- rotation\t" << _newRotationAngle << "\n"
             << "- pitch\t\t" << _newPitchPosition << "\n"
             << "- tilt\t\t" << _newTiltAngle << "\n";

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
            dynamic_cast<TubularGantry*>(system.gantries().front());
}

void TubularGantryParam::fromVariant(const QVariant &variant)
{
    QVariantMap varMap = variant.toMap();

    if(varMap.contains("rotation angle"))
        _newRotationAngle = { true, varMap.value("rotation angle").toDouble() };
    if(varMap.contains("pitch position"))
        _newPitchPosition = { true, varMap.value("pitch position").toDouble() };
    if(varMap.contains("tilt angle"))
        _newTiltAngle = { true, varMap.value("tilt angle").toDouble() };
}

QVariant TubularGantryParam::toVariant() const
{
    QVariantMap ret = AbstractPrepareStep::toVariant().toMap();

    if(_newRotationAngle.first)
        ret.insert("rotation angle",_newRotationAngle.second);
    if(_newPitchPosition.first)
        ret.insert("pitch position",_newPitchPosition.second);
    if(_newTiltAngle.first)
        ret.insert("tilt angle",_newTiltAngle.second);

    return ret;
}

void CarmGantryParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = static_cast<CarmGantry*>(system->gantry());

    qDebug() << "PrepareCarmGantry --- preparing gantry\n"
             << "- location\t" << _newLocation.first;
    qDebug() << (_newLocation.second.position.info() + _newLocation.second.rotation.info()).c_str()
             << "- span\t\t" << _newCarmSpan << "\n";

    if(_newLocation.first)
        gantryPtr->setLocation(_newLocation.second);
    if(_newCarmSpan.first)
        gantryPtr->setCarmSpan(_newCarmSpan.second);
}

bool CarmGantryParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<CarmGantry*>(system.gantries().front());
}

void CarmGantryParam::fromVariant(const QVariant &variant)
{
    QVariantMap varMap = variant.toMap();

    if(varMap.contains("location"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("location"));
        _newLocation = { true, loc };
    }
    if(varMap.contains("c-arm span"))
        _newCarmSpan = { true, varMap.value("c-arm span").toDouble() };
}

QVariant CarmGantryParam::toVariant() const
{
    QVariantMap ret = AbstractPrepareStep::toVariant().toMap();

    if(_newLocation.first)
        ret.insert("location",_newLocation.second.toVariant());
    if(_newCarmSpan.first)
        ret.insert("c-arm span",_newCarmSpan.second);

    return ret;
}

void GenericGantryParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = static_cast<GenericGantry*>(system->gantry());

    qDebug() << "PrepareGenericGantry --- preparing gantry\n"
             << "- detector location\t" << _newDetectorLocation.first;
    qDebug() << (_newDetectorLocation.second.position.info()
                 + _newDetectorLocation.second.rotation.info()).c_str()
             << "- X-ray source location \t" << _newSourceLocation.first;
    qDebug() << (_newSourceLocation.second.position.info()
                 + _newSourceLocation.second.rotation.info()).c_str();

    if(_newDetectorLocation.first)
        gantryPtr->setDetectorLocation(_newDetectorLocation.second);
    if(_newSourceLocation.first)
        gantryPtr->setSourceLocation(_newSourceLocation.second);
}

bool GenericGantryParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<GenericGantry*>(system.gantries().front());
}

void GenericGantryParam::fromVariant(const QVariant &variant)
{
    QVariantMap varMap = variant.toMap();

    if(varMap.contains("detector location"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("detector location"));
        _newDetectorLocation = { true, loc };
    }
    if(varMap.contains("source location"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("source location"));
        _newSourceLocation = { true, loc };
    }
}

QVariant GenericGantryParam::toVariant() const
{
    QVariantMap ret = AbstractPrepareStep::toVariant().toMap();

    if(_newDetectorLocation.first)
        ret.insert("detector location", _newDetectorLocation.second.toVariant());
    if(_newSourceLocation.first)
        ret.insert("source location", _newSourceLocation.second.toVariant());

    return ret;
}

void GantryDisplacementParam::prepare(SimpleCTsystem* system) const
{
    auto gantryPtr = system->gantry();

    qDebug() << "PrepareGantryDisplacements --- preparing gantry\n"
             << "- detector displacement\t" << _newDetectorDisplacement.first;
    qDebug() << (_newDetectorDisplacement.second.position.info()
                 + _newDetectorDisplacement.second.rotation.info()).c_str()
             << "- gantry displacement\t" << _newGantryDisplacement.first;
    qDebug() << (_newGantryDisplacement.second.position.info()
                 + _newGantryDisplacement.second.rotation.info()).c_str()
             << "- X-ray source location\t" << _newSourceDisplacement.first;
    qDebug() << (_newSourceDisplacement.second.position.info()
                 + _newSourceDisplacement.second.rotation.info()).c_str();

    if(_newDetectorDisplacement.first)
        gantryPtr->setDetectorDisplacement(_newDetectorDisplacement.second);
    if(_newGantryDisplacement.first)
        gantryPtr->setGantryDisplacement(_newGantryDisplacement.second);
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

void GantryDisplacementParam::fromVariant(const QVariant &variant)
{
    QVariantMap varMap = variant.toMap();

    if(varMap.contains("detector displacement"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("detector displacement"));
        _newDetectorDisplacement = { true, loc };
    }
    if(varMap.contains("gantry displacement"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("gantry displacement"));
        _newGantryDisplacement = { true, loc };
    }
    if(varMap.contains("source displacement"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("source displacement"));
        _newSourceDisplacement = { true, loc };
    }
    if(varMap.contains("detector displacement increment"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("detector displacement increment"));
        _detectorDisplacementIncrement = { true, loc };
    }
    if(varMap.contains("source displacement increment"))
    {
        mat::Location loc;
        loc.fromVariant(varMap.value("source displacement increment"));
        _sourceDisplacementIncrement = { true, loc };
    }
}

QVariant GantryDisplacementParam::toVariant() const
{
    QVariantMap ret = AbstractPrepareStep::toVariant().toMap();

    if(_newDetectorDisplacement.first)
        ret.insert("detector displacement", _newDetectorDisplacement.second.toVariant());
    if(_newGantryDisplacement.first)
        ret.insert("gantry displacement", _newGantryDisplacement.second.toVariant());
    if(_newSourceDisplacement.first)
        ret.insert("source displacement", _newSourceDisplacement.second.toVariant());
    if(_detectorDisplacementIncrement.first)
        ret.insert("detector displacement increment", _detectorDisplacementIncrement.second.toVariant());
    if(_sourceDisplacementIncrement.first)
        ret.insert("source displacement increment", _sourceDisplacementIncrement.second.toVariant());

    return ret;
}

// ### ### ### ###
// ### SOURCES ###
// ### ### ### ###

void SourceParam::prepare(SimpleCTsystem* system) const
{
    auto sourcePtr = system->source();

    qDebug() << "PrepareAbstractSource --- preparing source\n"
             << "- flux mod\t" << _newFluxModifier << "\n"
             << "- focal spot size\t" << _newFocalSpotSize << "\n"
             << "- focal spot pos\t" << _newSpotPosition.first;
    qDebug() << _newSpotPosition.second.info().c_str();

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

void SourceParam::fromVariant(const QVariant &variant)
{
    QVariantMap varMap = variant.toMap();

    if(varMap.contains("focal spot position"))
    {
        auto fsPos = varMap.value("focal spot position").toList();
        Vector3x1 fsPosVec(fsPos.at(0).toDouble(),
                           fsPos.at(1).toDouble(),
                           fsPos.at(2).toDouble());

        _newSpotPosition = { true, fsPosVec };
    }
    if(varMap.contains("focal spot size"))
    {
        auto fsSize = varMap.value("focal spot size").toMap();
        QSizeF fsQSize;
        fsQSize.setWidth(fsSize.value("width").toDouble());
        fsQSize.setHeight(fsSize.value("height").toDouble());

        _newFocalSpotSize = { true, fsQSize };
    }
    if(varMap.contains("flux modifier"))
        _newFluxModifier = { true, varMap.value("flux modifier").toDouble() };
}

QVariant SourceParam::toVariant() const
{
    QVariantMap ret = AbstractPrepareStep::toVariant().toMap();

    if(_newSpotPosition.first)
    {
        QVariantList fsPos;
        fsPos.append(_newSpotPosition.second.get<0>());
        fsPos.append(_newSpotPosition.second.get<1>());
        fsPos.append(_newSpotPosition.second.get<2>());
        ret.insert("focal spot position", fsPos);
    }
    if(_newFocalSpotSize.first)
    {
        QVariantMap fsSize;
        fsSize.insert("width", _newFocalSpotSize.second.width());
        fsSize.insert("height", _newFocalSpotSize.second.height());
        ret.insert("focal spot size", fsSize);
    }
    if(_newFluxModifier.first)
        ret.insert("flux modifier",_newFluxModifier.second);

    return ret;
}

void XrayLaserParam::prepare(SimpleCTsystem* system) const
{
    SourceParam::prepare(system);

    auto sourcePtr = static_cast<XrayLaser*>(system->source());

    qDebug() << "PrepareXrayLaser --- preparing source\n"
             << "- energy\t" << _newPhotonEnergy << "\n"
             << "- power\t" << _newPower << "\n";

    if(_newPhotonEnergy.first)
        sourcePtr->setPhotonEnergy(_newPhotonEnergy.second);
    if(_newPower.first)
        sourcePtr->setPower(_newPower.second);
}

bool XrayLaserParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<XrayLaser*>(system.sources().front());
}

void XrayLaserParam::fromVariant(const QVariant &variant)
{
    SourceParam::fromVariant(variant);

    QVariantMap varMap = variant.toMap();

    if(varMap.contains("photon energy"))
        _newPhotonEnergy = { true, varMap.value("photon energy").toDouble() };
    if(varMap.contains("power"))
        _newPower = { true, varMap.value("power").toDouble() };
}

QVariant XrayLaserParam::toVariant() const
{
    QVariantMap ret = SourceParam::toVariant().toMap();

    if(_newPhotonEnergy.first)
        ret.insert("photon energy",_newPhotonEnergy.second);
    if(_newPower.first)
        ret.insert("power",_newPower.second);

    return ret;
}

void XrayTubeParam::prepare(SimpleCTsystem* system) const
{
    SourceParam::prepare(system);

    auto sourcePtr = static_cast<XrayTube*>(system->source());

    qDebug() << "PrepareXrayTube --- preparing source\n"
             << "- voltage\t" << _newTubeVoltage << "\n"
             << "- emission\t" << _newEmissionCurrent << "\n";

    if(_newTubeVoltage.first)
        sourcePtr->setTubeVoltage(_newTubeVoltage.second);
    if(_newEmissionCurrent.first)
        sourcePtr->setEmissionCurrent(_newEmissionCurrent.second);
}

bool XrayTubeParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<XrayTube*>(system.sources().front());
}

void XrayTubeParam::fromVariant(const QVariant &variant)
{
    SourceParam::fromVariant(variant);

    QVariantMap varMap = variant.toMap();

    if(varMap.contains("tube voltage"))
        _newTubeVoltage = { true, varMap.value("tube voltage").toDouble() };
    if(varMap.contains("emission current"))
        _newEmissionCurrent = { true, varMap.value("emission current").toDouble() };
}

QVariant XrayTubeParam::toVariant() const
{
    QVariantMap ret = SourceParam::toVariant().toMap();

    if(_newTubeVoltage.first)
        ret.insert("tube voltage",_newTubeVoltage.second);
    if(_newEmissionCurrent.first)
        ret.insert("emission current",_newEmissionCurrent.second);

    return ret;
}

void GenericDetectorParam::prepare(SimpleCTsystem* system) const
{
    auto detectorPtr = static_cast<GenericDetector*>(system->detector());

    qDebug() << "PrepareGenericDetector --- preparing detector\n"
             << "- module locations\t" << _newModuleLocations.first << "\n"
             << "- number of modules\t" << _newModuleLocations.second.size() << "\n"
             << "- pixelSize\t" << _newPixelSize << "\n"
             << "- skew\t" << _newSkewCoefficient << "\n";

    if(_newModuleLocations.first)
        detectorPtr->setModuleLocations(_newModuleLocations.second);
    if(_newPixelSize.first)
        detectorPtr->setPixelSize(_newPixelSize.second);
    if(_newSkewCoefficient.first)
        detectorPtr->setSkewCoefficient(_newSkewCoefficient.second);
}

bool GenericDetectorParam::isApplicableTo(const CTsystem& system) const
{
    return system.isSimple() &&
           dynamic_cast<GenericDetector*>(system.detectors().front());
}

void GenericDetectorParam::fromVariant(const QVariant &variant)
{
    QVariantMap varMap = variant.toMap();

    if(varMap.contains("module locations"))
    {
        QVariantList moduleList = varMap.value("module locations").toList();
        QVector<mat::Location> modLocs;
        modLocs.reserve(moduleList.size());
        for(const auto& mod : moduleList)
        {
            mat::Location loc;
            loc.fromVariant(mod);
            modLocs.append(loc);
        }

        _newModuleLocations = { true, std::move(modLocs) };
    }
    if(varMap.contains("pixel size"))
    {
        auto pixSize = varMap.value("pixel size").toMap();
        QSizeF pixQSize;
        pixQSize.setWidth(pixSize.value("width").toDouble());
        pixQSize.setHeight(pixSize.value("height").toDouble());

        _newPixelSize = { true, pixQSize };
    }
    if(varMap.contains("skew coefficient"))
        _newSkewCoefficient = { true, varMap.value("skew coefficient").toDouble() };
}

QVariant GenericDetectorParam::toVariant() const
{
    QVariantMap ret;

    if(_newModuleLocations.first)
    {
        QVariantList moduleList;
        moduleList.reserve(_newModuleLocations.second.size());
        for(const auto& mod : _newModuleLocations.second)
            moduleList.append(mod.toVariant());
        ret.insert("module locations", moduleList);
    }
    if(_newPixelSize.first)
    {
        QVariantMap pixSize;
        pixSize.insert("width", _newPixelSize.second.width());
        pixSize.insert("height", _newPixelSize.second.height());
        ret.insert("pixel size", pixSize);
    }
    if(_newSkewCoefficient.first)
        ret.insert("skew coefficient",_newSkewCoefficient.second);

    return ret;
}


} // namespace prepare
} // namespace CTL
