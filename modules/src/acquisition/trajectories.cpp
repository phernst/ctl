#include "preparesteps.h"
#include "trajectories.h"
#include "acquisitionsetup.h"
#include "components/carmgantry.h"
#include "components/cylindricaldetector.h"
#include "components/flatpaneldetector.h"

#include <QDebug>

namespace CTL {
namespace protocols {


HelicalTrajectory::HelicalTrajectory(double angleIncrement,
                                     double pitchIncrement,
                                     double startPitch,
                                     double startAngle)
    : _angleIncrement(angleIncrement)
    , _pitchIncrement(pitchIncrement)
    , _startPitch(startPitch)
    , _startAngle(startAngle)
{
}

void HelicalTrajectory::setAngleIncrement(double angleIncrement) { _angleIncrement = angleIncrement; }

void HelicalTrajectory::setPitchIncrement(double pitchIncrement) { _pitchIncrement = pitchIncrement; }

void HelicalTrajectory::setStartPitch(double startPitch) { _startPitch = startPitch; }

void HelicalTrajectory::setStartAngle(double startAngle) { _startAngle = startAngle; }

std::vector<std::shared_ptr<AbstractPrepareStep>>
HelicalTrajectory::prepareSteps(uint viewNb, const AcquisitionSetup&) const
{
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    auto gantryPrep = std::make_shared<prepare::TubularGantryParam>();

    gantryPrep->setPitchPosition(viewNb * _pitchIncrement + _startPitch);
    gantryPrep->setRotationAngle(viewNb * _angleIncrement + _startAngle);

    ret.push_back(std::move(gantryPrep));

    qDebug() << "HelicalTrajectory --- add prepare steps for view: " << viewNb
             << "\n-rotation: " << viewNb * _angleIncrement + _startAngle
             << "\n-pitch: " << viewNb * _pitchIncrement + _startPitch;

    return ret;
}

bool HelicalTrajectory::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::TubularGantryParam tmp;
    return tmp.isApplicableTo(*setup.system());
}

WobbleTrajectory::WobbleTrajectory(double angleSpan,
                                   double sourceToIsocenter,
                                   double startAngle,
                                   double wobbleAngle,
                                   double wobbleFreq)
    : _angleSpan(angleSpan)
    , _sourceToIsocenter(sourceToIsocenter)
    , _startAngle(startAngle)
    , _wobbleAngle(wobbleAngle)
    , _wobbleFreq(wobbleFreq)
{
}

std::vector<std::shared_ptr<AbstractPrepareStep>>
WobbleTrajectory::prepareSteps(uint viewNb, const AcquisitionSetup& setup) const
{
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    auto gantryPrep = std::make_shared<prepare::CarmGantryParam>();

    const uint nbViews = setup.nbViews();
    const Vector3x1 initialSrcPos(0.0, 0.0, -_sourceToIsocenter);
    const Matrix3x3 fixedRotMat = mat::rotationMatrix(PI_2 + _startAngle, Qt::ZAxis)
        * mat::rotationMatrix(-PI_2, Qt::XAxis);
    const double angleIncrement = (nbViews > 1) ? _angleSpan / double(nbViews - 1) : 0.0;

    const auto wobblePhase = std::sin(double(viewNb) / double(nbViews) * 2.0 * PI * _wobbleFreq);
    const auto wobbleRotMat = mat::rotationMatrix(wobblePhase * _wobbleAngle, Qt::XAxis);

    const auto viewRotation
        = mat::rotationMatrix(viewNb * angleIncrement, Qt::ZAxis) * fixedRotMat * wobbleRotMat;
    const auto viewPosition = viewRotation * initialSrcPos;

    gantryPrep->setLocation(mat::Location(viewPosition, viewRotation));

    ret.push_back(std::move(gantryPrep));

    return ret;
}

bool WobbleTrajectory::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::CarmGantryParam tmp;
    return tmp.isApplicableTo(*setup.system());
}

void WobbleTrajectory::setStartAngle(double startAngle) { _startAngle = startAngle; }

void WobbleTrajectory::setWobbleAngle(double wobbleAngle) { _wobbleAngle = wobbleAngle; }

void WobbleTrajectory::setWobbleFreq(double wobbleFreq) { _wobbleFreq = wobbleFreq; }

CirclePlusLineTrajectory::CirclePlusLineTrajectory(double angleSpan,
                                                   double sourceToIsocenter,
                                                   double lineLength,
                                                   double fractionOfViewsForLine,
                                                   double startAngle)
    : _angleSpan(angleSpan)
    , _sourceToIsocenter(sourceToIsocenter)
    , _lineLength(lineLength)
    , _fractionOfViewsForLine(fractionOfViewsForLine)
    , _startAngle(startAngle)
{
}

std::vector<std::shared_ptr<AbstractPrepareStep>>
CirclePlusLineTrajectory::prepareSteps(uint viewNb, const AcquisitionSetup& setup) const
{
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    auto gantryPrep = std::make_shared<prepare::CarmGantryParam>();

    const uint nbViews = setup.nbViews();
    const uint nbViewsLine   = static_cast<uint>(std::floor(nbViews * _fractionOfViewsForLine));
    const uint nbViewsCircle = nbViews - nbViewsLine;
    const Vector3x1 initialSrcPos(0.0, 0.0, -_sourceToIsocenter);
    const Matrix3x3 fixedRotMat = mat::rotationMatrix(PI_2 + _startAngle, Qt::ZAxis)
        * mat::rotationMatrix(-PI_2, Qt::XAxis);

    Vector3x1 viewPosition;
    Matrix3x3 viewRotation;

    if(viewNb < nbViewsCircle)
    {
        const double angleIncrement = (nbViewsCircle > 1) ? _angleSpan / double(nbViewsCircle - 1) : 0.0;

        viewRotation = mat::rotationMatrix(viewNb * angleIncrement, Qt::ZAxis) * fixedRotMat;
        viewPosition = viewRotation * initialSrcPos;
    }
    else
    {
        const uint lineViewNb = viewNb - nbViewsCircle;
        const double lineIncrement = (nbViewsLine > 1) ? _lineLength / double(nbViewsLine - 1) : 0.0;

        viewRotation = mat::rotationMatrix(_angleSpan/2.0, Qt::ZAxis) * fixedRotMat;
        viewPosition = viewRotation * initialSrcPos;
        viewPosition.get<2>() += (lineViewNb - nbViewsLine/2.0) * lineIncrement;
    }

    gantryPrep->setLocation(mat::Location(viewPosition, viewRotation));

    ret.push_back(std::move(gantryPrep));

    return ret;
}

bool CirclePlusLineTrajectory::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::CarmGantryParam tmp;
    return tmp.isApplicableTo(*setup.system());
}

ShortScanTrajectory::ShortScanTrajectory(double sourceToIsocenter, double startAngle, double angleSpan)
    : _sourceToIsocenter(sourceToIsocenter)
    , _startAngle(startAngle)
    , _angleSpan(angleSpan)
{
}

std::vector<std::shared_ptr<AbstractPrepareStep> > ShortScanTrajectory::prepareSteps(uint viewNb, const AcquisitionSetup &setup) const
{
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    auto gantryPrep = std::make_shared<prepare::CarmGantryParam>();

    double angleSpan;
    if(_angleSpan >= 0.0)
        angleSpan = _angleSpan;
    else
        angleSpan = 180.0_deg + fanAngle(setup);

    qDebug() << "short scan angle span: " << angleSpan;

    const uint nbViews = setup.nbViews();
    const Vector3x1 initialSrcPos(0.0, 0.0, -_sourceToIsocenter);
    const Matrix3x3 fixedRotMat = mat::rotationMatrix(PI_2 + _startAngle, Qt::ZAxis)
        * mat::rotationMatrix(-PI_2, Qt::XAxis);
    const double angleIncrement = (nbViews > 1) ? angleSpan / double(nbViews - 1) : 0.0;

    const auto viewRotation = mat::rotationMatrix(viewNb * angleIncrement, Qt::ZAxis) * fixedRotMat;
    const auto viewPosition = viewRotation * initialSrcPos;

    gantryPrep->setLocation(mat::Location(viewPosition, viewRotation));

    ret.push_back(std::move(gantryPrep));

    return ret;
}

bool ShortScanTrajectory::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::CarmGantryParam tmp;
    return tmp.isApplicableTo(*setup.system());
}

double ShortScanTrajectory::fanAngle(const AcquisitionSetup& setup) const
{
    auto detector = setup.system()->detector();
    auto gantry   = static_cast<CarmGantry*>(setup.system()->gantry());

    double relevantWidth;
    switch (detector->type()) {
    case CylindricalDetector::Type:
    {
        const auto cylDetPtr = static_cast<CylindricalDetector*>(detector);
        const auto cylFan = cylDetPtr->fanAngle();
        relevantWidth = 2.0 * cylDetPtr->curvatureRadius() * std::sin(cylFan / 2.0);
        break;
    }
    case FlatPanelDetector::Type:
    {
        relevantWidth = static_cast<FlatPanelDetector*>(detector)->panelDimensions().width();
        break;
    }
    default:
        relevantWidth = 0.0;
        break;
    }

    return 2.0 * tan(0.5*relevantWidth / gantry->cArmSpan());
}

AxialScanTrajectory::AxialScanTrajectory(double startAngle)
    : _startAngle(startAngle)
{
}

std::vector<std::shared_ptr<AbstractPrepareStep>>
AxialScanTrajectory::prepareSteps(uint viewNb, const AcquisitionSetup& setup) const
{
    float angleIncrement = (setup.nbViews() > 0) ? 360.0_deg / setup.nbViews()
                                                 : 0.0;
    std::vector<std::shared_ptr<AbstractPrepareStep>> ret;

    auto gantryPrep = std::make_shared<prepare::TubularGantryParam>();

    gantryPrep->setRotationAngle(viewNb * angleIncrement + _startAngle);

    ret.push_back(std::move(gantryPrep));

    qDebug() << "AxialScanTrajectory --- add prepare steps for view: " << viewNb
             << "\n-rotation: " << viewNb * angleIncrement + _startAngle;

    return ret;
}

bool AxialScanTrajectory::isApplicableTo(const AcquisitionSetup &setup) const
{
    prepare::TubularGantryParam tmp;
    return tmp.isApplicableTo(*setup.system());
}

void AxialScanTrajectory::setStartAngle(double startAngle) { _startAngle = startAngle; }

} // namespace protocols
} // namespace CTL
