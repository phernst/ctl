#include "acquisitionsetupview.h"
#include "acquisition/acquisitionsetup.h"
#include <QTimer>
#include <cmath>

namespace CTL {
namespace gui {

/*!
 * Creates an AcquisitionSetupView and sets its parent to \a parent.
 *
 * If specified, sets the scaling for the visual appearance of components within the scene to
 * \a visualScale.
 */
AcquisitionSetupView::AcquisitionSetupView(QWidget* parent, float visualScale)
    : CTSystemView(parent, visualScale)
    , _animTimer(new QTimer(this))
    , _animCurrentView(0)
    , _animLeaveOut(0)
{
    connect(_animTimer, SIGNAL(timeout()), SLOT(updateAnimation()));

    resize(800, 600);

    resize(500,400);

    setWindowTitle("Acquisition setup view");
}

/*!
 * Sets the AcquisitionSetup visualized by this instance to \a acqSetup; the passed setup is copied.
 * This replaces any previously set system.
 *
 * By default, this shows the visualization of the setup's configuration prepared for the first view
 * in \a acqSetup. To change the visualization of the system, use one of the following methods:
 * showView(), addViewVisualization(), showFullAcquisition(), showSourceTrajectory(), or
 * animateAcquisition().
 */
void AcquisitionSetupView::setAcquisitionSetup(const AcquisitionSetup& acqSetup)
{
    _setup = acqSetup;

    if(_setup.nbViews())
        showView(0);
}

/*!
 * Overload of setAcquisitionSetup(const AcquisitionSetup&) for r-value references.
 */
void AcquisitionSetupView::setAcquisitionSetup(AcquisitionSetup&& acqSetup)
{
    _setup = std::move(acqSetup);

    if(_setup.nbViews())
        showView(0);
}

/*!
 * Creates an AcquisitionSetupView for visualization of \a setup and shows the window.
 *
 * The visualization will show a superposition of all views in \a setup. The total number of views
 * shown in the scene can be limited by \a maxNbViews. If desired, only the source positions can be
 * drawn by passing \c true to \a sourceOnly. The scaling for the visual appearance of components
 * within the scene can be changed with \a visualScale (see CTSystemView::plot() for an example).
 *
 * The widget will be deleted automatically if the window is closed.
 *
 * Example: create visualizations of a (100 view) short scan trajectory with a C-arm system
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * AcquisitionSetup setup(system, 100);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(600.0));
 *
 * gui::AcquisitionSetupView::plot(setup);     // show all views of the setup (here: 100)
 * gui::AcquisitionSetupView::plot(setup, 10); // limit number of displayed views to 10
 * \endcode
 *
 * ![Resulting visualization from the example above (window size and zoom adjusted).](gui/AcquisitionSetupView_plot.png)
 */
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

/*!
 * Adds the visualization of the setup in its configuration for the view index \a view to the scene.
 * If "Source only" (see setSourceOnly()) mode has been enabled, only the source component will be
 * drawn.
 *
 * Example: create a visualization of two views from a short scan trajectory with a C-arm system
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * AcquisitionSetup setup(system, 100);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(600.0));
 *
 * auto viewer = new gui::AcquisitionSetupView;
 * viewer->setAcquisitionSetup(setup);  // by default, shows view 0
 * viewer->addViewVisualization(30);    // add view 30 to the scene
 * viewer->show();
 * \endcode
 *
 * ![Resulting visualization from the example above (window size and zoom adjusted).](gui/AcquisitionSetupView_add.png)
 */
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

/*!
 * Shows an animation of the current acquisition setup of this instance. This will draw a
 * visualization of one view from the setup every \a msPerView milliseconds. If required, views can
 * be skipped by passing to \a leaveOut the desired number of views to be skipped in between two
 * visualized configurations.
 *
 * The total animation time will be
 * \f$ t = \left \lfloor{(N / (1 + leaveOut))}\right \rfloor\cdot msPerView \f$, where \f$N\f$
 * denotes the total number of views in the setup.
 *
 * All settings made for this instance apply to this command. In particular, that means if "Source
 * only" mode has been enabled, only the source component will appear in the animation; and in case
 * "Animation stacking" has been enabled, all system configurations are superimposed in the scene.
 *
 * Note that you still need to show() the widget.
 *
 * Example: create an animation of a wobble scan trajectory with a C-arm system
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * AcquisitionSetup setup(system, 100);
 * setup.applyPreparationProtocol(protocols::WobbleTrajectory(200.0_deg, 600.0, 180.0_deg, 30.0_deg, 2.0f));
 *
 * auto viewer = new gui::AcquisitionSetupView;
 * viewer->setAcquisitionSetup(setup);
 * viewer->show();
 * viewer->animateAcquisition(50);
 * \endcode
 */
void AcquisitionSetupView::animateAcquisition(int msPerView, uint leaveOut)
{
    if(!_setup.isValid())
        return;

    clearScene();

    _animCurrentView = 0;
    _animLeaveOut = leaveOut;
    _animTimer->start(msPerView);
}

/*!
 * Sets the "Animation stacking" mode to \a enabled. When enabled before calling
 * animateAcquisition(), this mode causes all system configurations to be superimposed in the
 * animation's scene.
 */
void AcquisitionSetupView::setAnimationStacking(bool enabled)
{
    _stackAnimation = enabled;
}

/*!
 * Sets the "Source only" mode to \a enabled. In "Source only" mode, all visualization commands will
 * only show the source component of the system. This might be useful to prevent cluttered scenes.
 *
 * Example: helical trajectory with and without "Source only" mode
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericTubularCT());
 * AcquisitionSetup setup(system, 200);
 * setup.applyPreparationProtocol(protocols::HelicalTrajectory(7.0_deg, -2.0, 200.0));
 *
 * auto viewer = new gui::AcquisitionSetupView;
 * viewer->setAcquisitionSetup(setup);
 * viewer->show();
 *
 * // viewer->setSourceOnly(true); // uncomment to enable "Source only" mode
 * viewer->showFullAcquisition();  // shows source and detector
 *
 * // the same can be achieved with the plot() command (static-approach)
 * gui::AcquisitionSetupView::plot(setup, 200);       // all visible
 * gui::AcquisitionSetupView::plot(setup, 200, true); // only sources shown
 * \endcode
 *
 * ![Visualization of a helical trajectory with and without "Source only" mode ((a)-(d) window size and zoom adjusted; (c) and (d) camera moved).](gui/AcquisitionSetupView_sourceOnly.png)
 */
void AcquisitionSetupView::setSourceOnly(bool enabled)
{
    _sourceOnly = enabled;
}

/*!
 * Visualizes the current acquisition setup of this instance as a superposition of all views from
 * the setup. If required, views can be left out by passing to \a leaveOut the desired number of
 * views to be skipped in between two visualized configurations.
 *
 * The total number of configurations shown in the scene will be
 * \f$ \left \lfloor{(N / (1 + leaveOut))}\right \rfloor \f$, where \f$N\f$ denotes the total number
 * of views in the setup.
 *
 * "Source only" mode applies to this command. See setSourceOnly() for details.
 *
 * Note that you still need to show() the widget.
 *
 * Example: visualize a wobble scan trajectory of a C-arm system
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * AcquisitionSetup setup(system, 100);
 * setup.applyPreparationProtocol(protocols::WobbleTrajectory(200.0_deg, 600.0, 180.0_deg, 30.0_deg, 2.0f));
 *
 * auto viewer = new gui::AcquisitionSetupView;
 * viewer->setAcquisitionSetup(setup);
 * viewer->showFullAcquisition();    // shows all views
 * //viewer->showFullAcquisition(3); // leaves out 3 views between two shown configurations
 * viewer->show();
 * \endcode
 *
 * ![Visualization with different number of views left out).](gui/AcquisitionSetupView_fullAcq.png)
 */
void AcquisitionSetupView::showFullAcquisition(uint leaveOut)
{
    clearScene();

    for(uint view = 0; view < _setup.nbViews(); (++view) += leaveOut)
        addViewVisualization(view);
}

/*!
 * Visualizes the current acquisition setup of this instance in "Source only" mode.
 *
 * Example:
 * See setSourceOnly().
 */
void AcquisitionSetupView::showSourceTrajectory()
{
    const auto cache = _sourceOnly;

    setSourceOnly(true);   // (temporarily) enforce "source only" mode
    showFullAcquisition();
    setSourceOnly(cache);  // restore setting
}

/*!
 * Visualizes the system configuration prepared for view index \a view. This replaces all previous
 * visualizations in the scene.
 *
 * "Source only" mode applies to this command. See setSourceOnly() for details.
 */
void AcquisitionSetupView::showView(int view)
{
    clearScene();

    addViewVisualization(view);
}

/*!
 * Connected to the animation timer. Everytime the timer times out, this prepares the next view
 * (increment depending on \c _animLeaveOut) and initiates the appropriate visualization; this
 * considers all settings such as "Source only" mode and "Animation stacking".
 */
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

    _animCurrentView += (1 + _animLeaveOut);
}

} // namespace gui
} // namespace CTL
