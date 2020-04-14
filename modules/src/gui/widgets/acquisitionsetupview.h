#ifndef CTL_ACQUISITIONSETUPVIEW_H
#define CTL_ACQUISITIONSETUPVIEW_H

#include "ctsystemview.h"
#include "acquisition/acquisitionsetup.h"

// forward declarations
class QTimer;

namespace CTL {
namespace gui {

class AcquisitionSetupView : public CTSystemView
{
    Q_OBJECT

public:
    explicit AcquisitionSetupView(QWidget* parent = nullptr, float visualScale = 50.0f);

    void setAcquisitionSetup(const AcquisitionSetup& acqSetup);
    void setAcquisitionSetup(AcquisitionSetup&& acqSetup);

    static void plot(AcquisitionSetup setup, uint maxNbViews = 100, bool sourceOnly = false,
                     float visualScale = 50.0f);

public slots:
    void addViewVisualization(int view);
    void animateAcquisition(int msPerView);
    void setAnimationStacking(bool enabled);
    void setSourceOnly(bool enabled);
    void showFullAcquisition(uint leaveOut = 0);
    void showSourceTrajectory();
    void showView(int view);

private:
    AcquisitionSetup _setup;
    QTimer* _animTimer;
    uint _animCurrentView;

    bool _stackAnimation = false;
    bool _sourceOnly = false;

private slots:
    void updateAnimation();
};

} // namespace gui
} // namespace CTL

#endif // CTL_ACQUISITIONSETUPVIEW_H
