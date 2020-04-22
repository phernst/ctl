#ifndef CTL_ACQUISITIONSETUP_H
#define CTL_ACQUISITIONSETUP_H

#include "abstractpreparestep.h"
#include "simplectsystem.h"

namespace CTL {

/*!
 * \class AcquisitionSetup
 *
 * \brief Holds a CTSystem together with the information about the system settings for all views
 * from which projection images shall be simulated.
 *
 * The AcquisitionSetup class manages the CTSystem used during an acquisition, i.e. the simulation
 * of multiple views with differing conditions (e.g. varying geometry). The system to be used in the
 * acquisition is either set in the constructor or via resetSystem(). To specify the conditions for
 * all views in the acquisition, three approaches can be used: making use of preparation protocols,
 * specifying each view individually, or using a combination of both. These approaches are explained
 * in full detail at the end of this description (see *How to configure the views:*).
 *
 * To bring the system managed by the setup into the state for a certain view, use prepareView().
 * This will apply all preparation steps associated with that particular view.
 *
 * When using the 'ctl_qtgui.pri' module (or the submodule 'gui_widgets_3d.pri') within the project,
 * you can use the gui::AcquisitionSetupView class to visualize the setup.
 *
 * Before using an AcquisitionSetup, the method isValid() can be used to check whether the current
 * configuration is ready to use. Unless isValid() is \c true, using the setup in a simulation,
 * calling prepareView() on it, or trying to visualize the setup, might cause an exception.
 *
 * *How to configure the views:*
 * 1. Using preparation protocols: preparation protocols are a convenient way of specifying conditions for
 * the whole acquistion in a single step. The protocols can describe many different things, such as
 * geometry information (i.e. the acquisition trajectory) or dose modulation effects.
 * To use a preparation protocol in your acquisition, first specify the number of views that the
 * acquisition shall contain (either directly in the constructor or via setNbViews()). Aftwerwards,
 * apply the desired protocol to your setup using applyPreparationProtocol(). You might want to
 * check whether the protocol can be used with the system specified in your setup before trying to
 * apply it using AbstractPreparationProtocol::isApplicableTo().
 * In the following example, we create a setup that uses a GenericTubularCT (i.e. it has a tubular
 * gantry) and investigate two different trajectories, namely a helical scan trajectory and a wobble
 * trajectory. The wobble trajectory cannot be realized by a tubular gantry, as it lacks the degree
 * of freedom required to do so, and thus, we should not be able to use this combination:
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericTubularCT());
 *
 * AcquisitionSetup setup(system);
 * setup.setNbViews(100);
 *
 * auto protocol1 = protocols::WobbleTrajectory(200.0_deg, 500.0);
 * auto protocol2 = protocols::HelicalTrajectory(1.0_deg, 1.0);
 *
 * qInfo() << protocol1.isApplicableTo(setup);   // output: false
 * qInfo() << protocol2.isApplicableTo(setup);   // output: true
 *
 * // This tells us, we are not allowed to do this:
 * // setup.applyPreparationProtocol(protocol1); // this would be improper use...
 * // setup.prepareView(10);                     // ...because it results in undefined behavior here.
 *
 * // But we can do:
 * setup.applyPreparationProtocol(protocol2);
 *
 * // If you have included the 'ctl_qtgui.pri' module (or submodule 'gui_widgets_3d.pri') in the project,
 * // you can use the AcquisitionSetupView class to visualize the setup.
 * gui::AcquisitionSetupView::plot(setup);       // requires 'gui_widgets_3d.pri' submodule
 * \endcode
 * 2. Adding views individually: Instead of a preparation protocol that describes conditions for all
 * views in an acquisition at once, each view can be configured individually and added to the setup.
 * An AcquisitionSetup::View object must contain all preparation steps required to put the system in
 * the state that shall be used for the simulation of the corresponding view. After creation,
 * prepare steps can be added to a view with addPrepareStep(). A view also holds the time point it
 * corresponds to (used e.g. when projecting dynamic data). The time point is set either directly in
 * the constructor of View or using setTimeStamp(). When configured as desired, the view can be
 * added to the setup using addView(). Note that in case of individual adding of views, the number
 * of views in the setup must not be set in advance, as it is automatically increased each time
 * addView() is used (you would end up with many empty views that were created when setNbViews() was
 * used).
 * The following example demonstrates the creation of views with changing parameters for the C-arm
 * gantry used in the system:
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 *
 * AcquisitionSetup setup(system);
 *
 * for(uint v = 0; v < 100; ++v)
 * {
 *     // create the prepare step for our Carm gantry and ...
 *     auto gantryPar = std::make_shared<prepare::CarmGantryParam>();
 *     // ... set the source position (line parallel to the z-axis)
 *     gantryPar->setLocation(mat::Location( { 500.0, 0.0, -500.0 + 10.0 * double(v) },
 *                                           mat::rotationMatrix(-90.0_deg, Qt::YAxis)));
 *     // ... reduce the span of the C-arm (i.e. source-detector distance) from view to view
 *     gantryPar->setCarmSpan(1000.0 - 4.0 * double(v));
 *
 *     AcquisitionSetup::View view;    // create a new view
 *     view.addPrepareStep(gantryPar); // add the prepare step to the view
 *     setup.addView(view);            // add the view to our final setup
 * }
 *
 * // If you have included the 'ctl_qtgui.pri' module (or submodule 'gui_widgets_3d.pri') in the project,
 * // you can use the AcquisitionSetupView class to visualize the setup.
 * gui::AcquisitionSetupView::plot(setup, 10);  // we show only 10 views for better visibility
 * \endcode
 * ![Visualization of the setup from the example. As intended, the source component moves along a line parallel to the *z*-axis and the source-detector distance decreases along the trajectory. Note that only 10 views (out of the full 100) are shown for better visibility.](example_AcquisitionSetup.png)
 * 3. Using a combination of 1. and 2.: After application of a preparation protocol to a setup, it
 * remains possible to add additional views as described in option 2. Besides that, it is further
 * possible to manipulate the views that have been created by the preparation protocol. The latter
 * might be useful if only slight adjustments need to be made to an otherwise suitable protocol.
 * In the following example, we take a look at a short scan trajectory with a C-arm system, where we
 * want to add the effect from the previous example of reducing the source-detector distance from
 * view to view:
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 *
 * AcquisitionSetup setup(system, 100);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(500.0));
 *
 * // go through all views in the setup and add a step to change the C-arm span
 * for(uint v = 0; v < setup.nbViews(); ++v)
 * {
 *     // create an additional prepare step for the Carm gantry and ...
 *     auto gantryPar = std::make_shared<prepare::CarmGantryParam>();
 *     // ... reduce the span of the C-arm (i.e. source-detector distance) from view to view
 *     gantryPar->setCarmSpan(1000.0 - 4.0 * double(v));
 *
 *     setup.view(v).addPrepareStep(gantryPar); // add the prepare step to the view
 * }
 *
 * // If you have included the 'ctl_qtgui.pri' module (or submodule 'gui_widgets_3d.pri') in the project,
 * // you can use the AcquisitionSetupView class to visualize the setup.
 * gui::AcquisitionSetupView::plot(setup, 10); // we show only 10 views for better visibility
 * \endcode
 * ![Visualization of the setup from the example. As intended, the source-detector distance decreases along the (short-scan) trajectory. Note that only 10 views (out of the full 100) are shown for better visibility.](example_AcquisitionSetup2.png)
 * We can also add prepare steps for other parts of the system, for example the X-ray tube. In this
 * example, we switch the tube voltage twice during the acquisition:
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * // note that a GenericCarmCT has a default X-ray tube voltage of 100 kV
 *
 * AcquisitionSetup setup(system, 101);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(500.0)); // irrelevant for this example
 *
 * // we create two prepare steps for the voltage switchings
 * auto xRaySwitch1 = std::make_shared<prepare::XrayTubeParam>();
 * auto xRaySwitch2 = std::make_shared<prepare::XrayTubeParam>();
 * xRaySwitch1->setTubeVoltage(70.0);  // switch one sets the voltage to 70 kV
 * xRaySwitch2->setTubeVoltage(100.0); // switch two sets the voltage back to 100 kV
 *
 * // we now add the prepare steps to the views where they shall be applied
 * setup.view(30).addPrepareStep(std::move(xrayPar1)); // the first switch shall occur at view 30
 * setup.view(50).addPrepareStep(std::move(xrayPar2)); // the second switch shall occur at view 50
 *
 * // we can now read out the voltages for all views (if we want to inspect them):
 * const auto tube = static_cast<XrayTube*>(setup.system()->source()); // we remember a pointer to the XRayTube in the system
 * XYDataSeries voltages;
 * for(uint v = 0; v < setup.nbViews(); ++v)
 * {
 *     setup.prepareView(v);                    // this prepares the system for view "v"
 *     voltages.append(v, tube->tubeVoltage()); // this reads the tube voltage and adds it to our data series
 * }
 *
 * // If you have included the 'ctl_qtgui.pri' module (or submodule 'gui_widgets_charts.pri') in the project,
 * // you can use the LineSeriesView class to visualize the voltages.
 * gui::LineSeriesView::plot(voltages, "View index", "Tube voltage [kV]");
 * \endcode
 * ![Visualization of the tube voltages from the example. As intended, the voltage drops to 70 kV at view 30 and goes back to its initial 100 kV at view 50.](example_AcquisitionSetup3.png)
 * Note that in this example, it is not necessary to set the tube voltage for each view between 30
 * and 50, because it will not be altered by other prepare steps. Hence, it is sufficient for our
 * purpose to just switch the voltage once (and then switch it back again). This can be different
 * if other prepare steps are in use that also change the tube settings (esp. when using multiple
 * preparation protocols, this is a likely option).
 */

/*!
 * \class AcquisitionSetup::View
 *
 * \brief Holds the information about the system settings for a particular view.
 *
 *
 */
class AcquisitionSetup final : public SerializationInterface
{
public:
    typedef std::shared_ptr<const AbstractPrepareStep> PrepareStep; //!< Alias for shared_ptr to const AbstractPrepareStep.

    class View : public SerializationInterface
    {
    public:
        // ctors
        View() = default;
        View(double time);

        // time
        void setTimeStamp(double timeStamp);
        double timeStamp() const;

        // prepare steps
        void addPrepareStep(PrepareStep step);
        void clearPrepareSteps();
        size_t nbPrepareSteps() const;
        const std::vector<PrepareStep>& prepareSteps() const;
        const PrepareStep& prepareStep(int prepareStepType, bool searchFromBack = true) const;
        int indexOfPrepareStep(int prepareStepType, bool searchFromBack = true) const;
        bool replacePrepareStep(int index, PrepareStep newPrepareStep);
        bool replacePrepareStep(PrepareStep newPrepareStep, bool searchFromBack = true);
        void removeAllPrepareSteps(int prepareStepType);
        void removeLastPrepareStep();
        void removePrepareStep(int prepareStepType, bool searchFromBack = true);

        void fromVariant(const QVariant& variant) override; // de-serialization
        QVariant toVariant() const override; // serialization

    private:
        PrepareStep& prepareStep(int prepareStepType, bool searchFromBack);

        double _timeStamp; //!< Time stamp of the view.
        std::vector<PrepareStep> _prepareSteps; //!< List of prepare steps to configure the view.
    };

    AcquisitionSetup() = default;
    AcquisitionSetup(const CTSystem& system, uint nbViews = 0);
    AcquisitionSetup(CTSystem&& system, uint nbViews = 0);
    AcquisitionSetup(std::unique_ptr<CTSystem> system, uint nbViews = 0);
    AcquisitionSetup(std::unique_ptr<SimpleCTSystem> system, uint nbViews = 0);
    AcquisitionSetup(uint nbViews);

    // cp/mv cstor/assignment
    AcquisitionSetup(const AcquisitionSetup& other);
    AcquisitionSetup& operator=(const AcquisitionSetup& other);
    AcquisitionSetup(AcquisitionSetup&& other) = default;
    AcquisitionSetup& operator=(AcquisitionSetup&& other) = default;

    void addView(View view);
    void applyPreparationProtocol(const AbstractPreparationProtocol& preparation);
    void clearViews(bool keepTimeStamps = false);
    bool isValid() const;
    uint nbViews() const;
    void prepareView(uint viewNb);
    void removeAllPrepareSteps();
    bool resetSystem(const CTSystem& system);
    bool resetSystem(CTSystem&& system);
    void setNbViews(uint nbViews);
    SimpleCTSystem* system();
    const SimpleCTSystem* system() const;
    View& view(uint viewNb);
    const View& view(uint viewNb) const;
    std::vector<View>& views();
    const std::vector<View>& views() const;

    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    std::unique_ptr<SimpleCTSystem> _system; //!< CTSystem used for the acquisition.
    std::vector<View> _views; //!< List of all views of the acquisition.
};

} // namespace CTL

#endif // CTL_ACQUISITIONSETUP_H
