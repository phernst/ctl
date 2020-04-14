#include "acquisitionsetupview.h"
#include "acquisition/acquisitionsetup.h"
#include <QTimer>
#include <cmath>

namespace CTL {
namespace gui {

AcquisitionSetupView::AcquisitionSetupView(QWidget* parent, float visualScale)
    : CTSystemView(parent, visualScale)
    , _animTimer(new QTimer(this))
    , _animCurrentView(0)
{
    connect(_animTimer, SIGNAL(timeout()), SLOT(updateAnimation()));

    resize(800, 600);
    setWindowTitle("Acquisition setup view");
}

void AcquisitionSetupView::setAcquisitionSetup(const AcquisitionSetup& acqSetup)
{
    _setup = acqSetup;
}

void AcquisitionSetupView::setAcquisitionSetup(AcquisitionSetup&& acqSetup)
{
    _setup = std::move(acqSetup);
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

void AcquisitionSetupView::addViewVisualization(int view)
{
    if(!_setup.isValid())
        return;

    if(view >= static_cast<int>(_setup.nbViews()))
    {
        qWarning("Requested view exceeds number of views in current acquisition setup.");
        return;
    }

    _setup.prepareView(view);
    _sourceOnly ? addSourceComponent(_setup.system()->gantry(),
                                     _setup.system()->source())
                : addSystemVisualization(*_setup.system());
}

void AcquisitionSetupView::animateAcquisition(int msPerView)
{
    if(!_setup.isValid())
        return;

    clearScene();

    _animCurrentView = 0;
    _animTimer->start(msPerView);
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
    clearScene();

    for(uint view = 0; view < _setup.nbViews(); (++view) += leaveOut)
        addViewVisualization(view);
}

void AcquisitionSetupView::showSourceTrajectory()
{
    const auto cache = _sourceOnly;

    _sourceOnly = true;
    showFullAcquisition();
    _sourceOnly = cache;
}

void AcquisitionSetupView::showView(int view)
{
    clearScene();

    addViewVisualization(view);
}

void AcquisitionSetupView::updateAnimation()
{
    if(_animCurrentView >= _setup.nbViews())
    {
        qDebug() << "animation stopped";
        _animTimer->stop();
        return;
    }

    qDebug() << "animate: " << _animCurrentView;

    _stackAnimation ? addViewVisualization(_animCurrentView)
                    : showView(_animCurrentView);

    ++_animCurrentView;
}

} // namespace gui
} // namespace CTL
