#include "acquisitionvisualizerwidget.h"
#include "acquisition/acquisitionsetup.h"
#include <QTimer>

AcquisitionVisualizerWidget::AcquisitionVisualizerWidget(QWidget* parent)
    : SystemVisualizerWidget(parent)
    , _animationTimer(new QTimer(this))
    , _currentView(0)
{
    connect(_animationTimer, SIGNAL(timeout()), SLOT(updateAnimation()));
}

void AcquisitionVisualizerWidget::setAcquisitionSetup(const CTL::AcquisitionSetup& acqSetup)
{
    _currentAcquisition = acqSetup;
}

void AcquisitionVisualizerWidget::setAcquisitionSetup(CTL::AcquisitionSetup &&acqSetup)
{
    _currentAcquisition = std::move(acqSetup);
}

void AcquisitionVisualizerWidget::animateAcquisition(int msPerView)
{
    if(!_currentAcquisition.isValid())
        return;

    clearScene();

    _currentView = 0;
    _animationTimer->start(msPerView);
}

void AcquisitionVisualizerWidget::setAnimationStacking(bool enabled)
{
    _stackAnimation = enabled;
}

void AcquisitionVisualizerWidget::setSourceOnly(bool enabled)
{
    _sourceOnly = enabled;
}

void AcquisitionVisualizerWidget::showFullAcquisition(uint leaveOut)
{
    if(!_currentAcquisition.isValid())
        return;

    clearScene();

    for(uint view = 0; view < _currentAcquisition.nbViews(); (++view) += leaveOut)
    {
        _currentAcquisition.prepareView(view);
        addSystemVisualization(*_currentAcquisition.system());
    }
}

void AcquisitionVisualizerWidget::showSourceTrajectory()
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

void AcquisitionVisualizerWidget::updateAnimation()
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
            visualizeSystem(*_currentAcquisition.system());
    }

    ++_currentView;
}


