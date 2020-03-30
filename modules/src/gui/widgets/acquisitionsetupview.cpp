#include "acquisitionsetupview.h"
#include "acquisition/acquisitionsetup.h"
#include <QTimer>
#include <cmath>

namespace CTL {
namespace gui {

AcquisitionSetupView::AcquisitionSetupView(QWidget* parent, float visualScale)
    : CTSystemView(parent, visualScale)
    , _animationTimer(new QTimer(this))
    , _currentView(0)
{
    connect(_animationTimer, SIGNAL(timeout()), SLOT(updateAnimation()));

    resize(800, 600);
    setWindowTitle("Acquisition setup view");
}

void AcquisitionSetupView::setAcquisitionSetup(const AcquisitionSetup& acqSetup)
{
    _currentAcquisition = acqSetup;
}

void AcquisitionSetupView::setAcquisitionSetup(AcquisitionSetup &&acqSetup)
{
    _currentAcquisition = std::move(acqSetup);
}

void AcquisitionSetupView::plot(AcquisitionSetup setup, uint maxNbViews, bool sourceOnly, float visualScale)
{
    const auto nbViews = setup.nbViews();

    auto viewer = new AcquisitionSetupView(nullptr, visualScale);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setAcquisitionSetup(std::move(setup));
    viewer->setSourceOnly(sourceOnly);

    auto reqLeaveOut = static_cast<uint>(std::ceil(float(nbViews) / float(maxNbViews))) - 1u;
    viewer->showFullAcquisition(reqLeaveOut);

    viewer->show();
}

void AcquisitionSetupView::animateAcquisition(int msPerView)
{
    if(!_currentAcquisition.isValid())
        return;

    clearScene();

    _currentView = 0;
    _animationTimer->start(msPerView);
}

void AcquisitionSetupView::setAnimationStacking(bool enabled)
{
    _stackAnimation = enabled;
}

void AcquisitionSetupView::setSourceOnly(bool enabled)
{
    _sourceOnly = enabled;
}

void AcquisitionSetupView::showFullAcquisition(uint leaveOut)
{
    if(!_currentAcquisition.isValid())
        return;

    clearScene();

    for(uint view = 0; view < _currentAcquisition.nbViews(); (++view) += leaveOut)
    {
        _currentAcquisition.prepareView(view);
        _sourceOnly ? addSourceComponent(_currentAcquisition.system()->gantry(),
                                         _currentAcquisition.system()->source())
                    : addSystemVisualization(*_currentAcquisition.system());
    }
}

void AcquisitionSetupView::showSourceTrajectory()
{
    if(!_currentAcquisition.isValid())
        return;

    clearScene();

    for(uint view = 0; view < _currentAcquisition.nbViews(); ++view)
    {
        _currentAcquisition.prepareView(view);
        addSourceComponent(_currentAcquisition.system()->gantry(),
                           _currentAcquisition.system()->source());
    }
}

void AcquisitionSetupView::updateAnimation()
{
    if(_currentView >= _currentAcquisition.nbViews())
    {
        _animationTimer->stop();
        return;
    }

    qDebug() << "animate: " << _currentView;
    _currentAcquisition.prepareView(_currentView);
    if(_stackAnimation)
    {
        if(_sourceOnly)
            addSourceComponent(_currentAcquisition.system()->gantry(),
                               _currentAcquisition.system()->source());
        else
            addSystemVisualization(*_currentAcquisition.system());
    }
    else
    {
        if(_sourceOnly)
        {
            clearScene();
            addSourceComponent(_currentAcquisition.system()->gantry(),
                               _currentAcquisition.system()->source());
        }
        else
            setCTSystem(*_currentAcquisition.system());
    }

    ++_currentView;
}

} // namespace gui
} // namespace CTL
