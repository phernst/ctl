#ifndef CTL_SYSTEMBLUEPRINTS_H
#define CTL_SYSTEMBLUEPRINTS_H

#include "components/allcomponents.h"
#include "acquisition/ctsystembuilder.h"
#include "mat/mat.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {

enum class DetectorBinning { Binning1x1, Binning2x2, Binning4x4 };

namespace blueprints {

class GenericTubularCT : public AbstractCTsystemBlueprint
{
public:
    AbstractDetector* detector() const override
    {
        return new CylindricalDetector(CylindricalDetector::fromRadiusAndFanAngle(QSize(16, 64),
                                                                                  QSizeF(1.2, 1.0),
                                                                                  40,
                                                                                  1000.0,
                                                                                  45.0_deg));
    }
    AbstractGantry* gantry() const override
    {
        return new TubularGantry(1000.0, 550.0);
    }
    AbstractSource* source() const override
    {
        return new XrayTube(QSizeF(1.0,1.0));
    }

    QString systemName() const override { return QStringLiteral("Tubular CT system"); }
};

class GenericCarmCT : public AbstractCTsystemBlueprint
{
public:
    GenericCarmCT(DetectorBinning binning = DetectorBinning::Binning2x2)
        : _binning(binning)
    {
    }

    AbstractDetector* detector() const override;
    AbstractGantry* gantry() const override
    {
        return new CarmGantry(1000.0, QStringLiteral("Robot arm"));
    }
    AbstractSource* source() const override
    {
        return new XrayTube(QSizeF(1.0,1.0));
    }

    QString systemName() const override { return QStringLiteral("C-arm CT system"); }

private:
    DetectorBinning _binning;
};

inline AbstractDetector* GenericCarmCT::detector() const
{
    QSize nbPixel;
    QSizeF pixelDimensions;
    QString binName;
    switch(_binning)
    {
    case DetectorBinning::Binning1x1:
        nbPixel = { 2560, 1920 };
        pixelDimensions = { 0.125, 0.125 };
        binName = QStringLiteral("1x1");
        break;
    case DetectorBinning::Binning2x2:
        nbPixel = { 1280, 960 };
        pixelDimensions = { 0.25, 0.25 };
        binName = QStringLiteral("2x2");
        break;
    case DetectorBinning::Binning4x4:
        nbPixel = { 640, 480 };
        pixelDimensions = { 0.5, 0.5 };
        binName = QStringLiteral("4x4");
        break;
    }
    auto detectorName = "flat panel with " + binName + "-binning";

    return new FlatPanelDetector(nbPixel, pixelDimensions, detectorName);
}

} // namespace blueprints

template <typename CTsystemBlueprint, typename... BlueprintCtorArgs>
std::unique_ptr<CTsystem> makeCTsystem(BlueprintCtorArgs&&... args)
{
    std::unique_ptr<CTsystem> ret{ CTsystemBuilder::createFromBlueprint(
                                       CTsystemBlueprint{
                                           std::forward<BlueprintCtorArgs>(args)... }).clone() };
    return ret;
}

template <typename CTsystemBlueprint, typename... BlueprintCtorArgs>
std::unique_ptr<SimpleCTsystem> makeSimpleCTsystem(BlueprintCtorArgs&&... args)
{
    std::unique_ptr<SimpleCTsystem> ret{ nullptr };
    bool canConvert;
    auto simpleSystem
        = SimpleCTsystem::fromCTsystem(CTsystemBuilder::createFromBlueprint(CTsystemBlueprint{
                                           std::forward<BlueprintCtorArgs>(args)... }),
                                       &canConvert);
    if(!canConvert)
        return ret;

    ret.reset(static_cast<SimpleCTsystem*>(std::move(simpleSystem).clone()));

    return ret;
}

} // namespace CTL

#endif // CTL_SYSTEMBLUEPRINTS_H
