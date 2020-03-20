#ifndef CTL_ACQUISITIONVISUALIZERWIDGET_H
#define CTL_ACQUISITIONVISUALIZERWIDGET_H

#include "systemvisualizerwidget.h"
#include"acquisition/acquisitionsetup.h"

// forward declarations
class QTimer;

class AcquisitionVisualizerWidget : public SystemVisualizerWidget
{
    Q_OBJECT

public:
    explicit AcquisitionVisualizerWidget(QWidget* parent = nullptr);

    void setAcquisitionSetup(const CTL::AcquisitionSetup& acqSetup);
    void setAcquisitionSetup(CTL::AcquisitionSetup&& acqSetup);

public slots:
    void animateAcquisition(int msPerView);
    void showFullAcquisition(uint leaveOut = 0);
    void showSourceTrajectory();

    void setAnimationStacking(bool enabled);
    void setSourceOnly(bool enabled);

private:
    CTL::AcquisitionSetup _currentAcquisition;
    QTimer* _animationTimer;
    uint _currentView;

    bool _stackAnimation = false;
    bool _sourceOnly = false;

private slots:
    void updateAnimation();
};

#endif // CTL_ACQUISITIONVISUALIZERWIDGET_H
