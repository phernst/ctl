#ifndef CTL_ACQUISITIONSETUPVIEW_H
#define CTL_ACQUISITIONSETUPVIEW_H

#include "ctsystemview.h"
#include "acquisition/acquisitionsetup.h"

// forward declarations
class QTimer;

namespace CTL {
namespace gui {

/*!
 * \class AcquisitionSetupView
 *
 * \brief The AcquisitionSetupView class provides a tool for visualization of an AcquisitionSetup.
 *
 * This class enhances the capabilities of CTSystemView, such that it can be used to visualize an
 * AcquisitionSetup. For convenience, the plot() method can be used to achieve a one-line solution,
 * creating a widget that will be destroyed once it is closed by the user.
 *
 * The following IO operations are supported by this widget:
 *
 * - Zooming:
 *    - Scroll mouse wheel up/down to zoom in/out.
 * - Camera positioning / orientation:
 *    - Hold left mouse button + move up/down/left/right to move the camera position in the
 * corresponding direction
 *    - Hold right mouse button + move up/down/left/right to rotate the camera direction
 *
 * The setup to be visualized is set via setAcquisitionSetup(). Visualization can be done in
 * two conceptually different ways: static or animated.
 * As a static visualization, the system configuration (ie. source and detector component) can be
 * shown simultaneously for all views in the setup (or with leaving out some views in between)
 * using showFullAcquisition(). It is also possible to visualize only the source component with
 * showSourceTrajectory().
 * The setup can also be visualized by an animation. Calling animateAcquisition() with a certain
 * time interval will show the system configuration for a single view and proceed to the next view
 * after the specified time interval has passed. You can enable animation stacking using
 * setAnimationStacking() to keep all previous system configurations in the scene during animation.
 * When setSourceOnly() is enabled, only the source component will be shown during animation of the
 * setup.
 *
 * Example:
 * \code
 * auto system = CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 *
 * // create an AcquisitionSetup with 100 views using the system we created
 * AcquisitionSetup setup(system, 100);
 * // we apply a circle-plus-line-trajectory with certain settings...
 * setup.applyPreparationProtocol(protocols::CirclePlusLineTrajectory(200.0_deg, 600.0, 500.0, 0.5, 180.0_deg));
 *
 * // ... and visualize the setup, showing a maximum of 30 views (for a better overview)
 * gui::AcquisitionSetupView::plot(setup, 30);
 *
 * // using the property-based approach, we can also create an animation of this setup
 * auto viewer = new gui::AcquisitionSetupView;
 * viewer->setAcquisitionSetup(setup);
 * viewer->show();
 * viewer->animateAcquisition(100); // show one view each 100 milliseconds (-> 10 FPS)
 * \endcode
 *
 * ![Visualization (static version) of the setup from the example.](gui/AcquisitionSetupView.png)
 */

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
    void animateAcquisition(int msPerView, uint leaveOut = 0);
    void setAnimationStacking(bool enabled);
    void setSourceOnly(bool enabled);
    void showFullAcquisition(uint leaveOut = 0);
    void showSourceTrajectory();
    void showView(int view);

private:
    AcquisitionSetup _setup;
    QTimer* _animTimer;
    uint _animCurrentView;
    uint _animLeaveOut;

    bool _stackAnimation = false;
    bool _sourceOnly = false;

private slots:
    void updateAnimation();
};

} // namespace gui
} // namespace CTL

#endif // CTL_ACQUISITIONSETUPVIEW_H
