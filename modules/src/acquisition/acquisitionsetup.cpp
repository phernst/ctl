#include <QDebug>

#include "acquisitionsetup.h"

namespace CTL {

/*!
 * Creates a View object and sets its time stamp to \a time.
 */
AcquisitionSetup::View::View(double time)
    : _timeStamp(time)
{
}

/*!
 * Sets the time stamp of this instance to \a timeStamp.
 */
void AcquisitionSetup::View::setTimeStamp(double timeStamp) { _timeStamp = timeStamp; }

/*!
 * Adds the PrepareStep \a step to the vector of prepare steps of this View. Prepare steps will
 * be applied in the same order as they have been added to the View.
 */
void AcquisitionSetup::View::addPrepareStep(PrepareStep step)
{
    if(step)
        _prepareSteps.push_back(std::move(step));
    else
        qWarning() << "AcquisitionSetup::View::addPrepareStep(): Prepare step not added! "
                      "Reason: tried to add 'nullptr'.";
}

/*!
 * Returns the time stamp of this instance.
 */
double AcquisitionSetup::View::timeStamp() const { return _timeStamp; }

/*!
 * Returns the number of prepare steps in this instance.
 */
size_t AcquisitionSetup::View::nbPrepareSteps() const { return _prepareSteps.size(); }

/*!
 * Returns a constant reference to the vector of prepare steps of this instance.
 */
const std::vector<AcquisitionSetup::PrepareStep>& AcquisitionSetup::View::prepareSteps() const
{
    return _prepareSteps;
}

/*!
 * Returns a constant reference to the first PrepareStep of type \a prepareStepType that occurs in
 * the vector of prepare steps in this instance. The \a prepareStepType refers to the type-id
 * provided by AbstractPrepareStep::type().
 *
 * If \a searchFromBack = \c true, the vector of prepare steps will be scanned through in reverse
 * order, thus, providing the PrepareStep of type \a prepareStepType occurring last in this
 * instance.
 *
 * Returns a reference to an invalid PrepareStep (wrapping a nullptr) if no PrepareStep of type
 * \a prepareStepType exists in this instance.
 */
const AcquisitionSetup::PrepareStep&
AcquisitionSetup::View::prepareStep(int prepareStepType, bool searchFromBack) const
{
    static PrepareStep invalid(nullptr);

    auto matchesType = [prepareStepType](const PrepareStep& prepStep)
    {
        return prepStep->type() == prepareStepType;
    };

    if(searchFromBack)
    {
        auto match = std::find_if(_prepareSteps.rbegin(), _prepareSteps.rend(), matchesType);
        return match == _prepareSteps.rend() ? invalid : *match;
    }
    else
    {
        auto match = std::find_if(_prepareSteps.begin(), _prepareSteps.end(), matchesType);
        return match == _prepareSteps.end() ? invalid : *match;
    }
}

/*!
 * Returns the index of the first PrepareStep of type \a prepareStepType that occurs in
 * the vector of prepare steps in this instance. The \a prepareStepType refers to the type-id
 * provided by AbstractPrepareStep::type().
 *
 * If \a searchFromBack = \c true, the vector of prepare steps will be scanned through in reverse
 * order, thus, providing the index of the PrepareStep of type \a prepareStepType occurring last
 * in this instance.
 *
 * Returns -1 if no PrepareStep of type \a prepareStepType exists in this instance.
 *
 * \sa AbstractPrepareStep::type().
 */
int AcquisitionSetup::View::indexOfPrepareStep(int prepareStepType, bool searchFromBack) const
{
    if(searchFromBack)
    {
        for(auto i = int(_prepareSteps.size()-1); i >= 0; --i)
            if(_prepareSteps[i]->type() == prepareStepType)
                return i;
    }
    else
    {
        for(auto i = 0, end = int(_prepareSteps.size()); i < end; ++i)
            if(_prepareSteps[i]->type() == prepareStepType)
                return i;
    }
    return -1;
}

/*!
 * Replaces the prepare step at position \a index in the vector of prepare steps by
 * \a newPrepareStep. Returns true if the prepare step has been replaced.
 *
 * The \a index must not exceed the range of the prepare step vector.
 *
 * Does nothing (and returns false) if \a index < 0 and/or \a newPrepareStep is nullptr.
 */
bool AcquisitionSetup::View::replacePrepareStep(int index, PrepareStep newPrepareStep)
{
    if(newPrepareStep == nullptr)
        return false;

    if(index < 0)
        return false;

    Q_ASSERT(size_t(index) < _prepareSteps.size());

    _prepareSteps[size_t(index)] = std::move(newPrepareStep);

    return true;
}

/*!
 * Replaces a prepare step of the same type as \a newPrepareStep by \a newPrepareStep. Returns
 * true if a prepare step has been replaced.
 *
 * This replaces the last occurrence of a corresponding PrepareStep. If
 * \a searchFromBack = \c false, the first matching element is replaced.
 *
 * Does nothing (and returns false) if no prepare step of matching type is found and/or
 * \a newPrepareStep is nullptr.
 */
bool AcquisitionSetup::View::replacePrepareStep(PrepareStep newPrepareStep, bool searchFromBack)
{
    if(newPrepareStep == nullptr)
        return false;

    auto& toBeReplaced = prepareStep(newPrepareStep->type(), searchFromBack);
    if(toBeReplaced == nullptr)
        return false;

    toBeReplaced = std::move(newPrepareStep);

    return true;
}

/*!
 * Removes all prepare steps of type \a prepareStepType from this instance.
 *
 * \sa AbstractPrepareStep::type().
 */
void AcquisitionSetup::View::removeAllPrepareSteps(int prepareStepType)
{
    std::vector<PrepareStep> newPrepareSteps;
    newPrepareSteps.reserve(_prepareSteps.size());

    for(auto& prepStep : _prepareSteps)
        if(prepStep->type() != prepareStepType)
            newPrepareSteps.push_back(std::move(prepStep));

    _prepareSteps = std::move(newPrepareSteps);
}

/*!
 * Removes the last prepare step from this instance.
 */
void AcquisitionSetup::View::removeLastPrepareStep()
{
    if(!_prepareSteps.empty())
        _prepareSteps.pop_back();
}

/*!
 * Removes one prepare step of type \a prepareStepType from this instance.
 *
 * This removes the last occurrence of a corresponding PrepareStep. If
 * \a searchFromBack = \c false, the first matching element is removed.
 *
 * \sa AbstractPrepareStep::type().
 */
void AcquisitionSetup::View::removePrepareStep(int prepareStepType, bool searchFromBack)
{
    auto idx = indexOfPrepareStep(prepareStepType, searchFromBack);

    if(idx == -1)
        return;

    _prepareSteps.erase(_prepareSteps.begin() + idx);
}

/*!
 * Removes all prepare steps from this instance. This keeps the time stamp of this instance
 * untouched.
 */
void AcquisitionSetup::View::clearPrepareSteps() { _prepareSteps.clear(); }

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void AcquisitionSetup::View::fromVariant(const QVariant& variant)
{
    QVariantMap varMap = variant.toMap();

    QVariantList prepareStepVarList = varMap.value("prepare steps").toList();
    std::vector<PrepareStep> prepSteps;
    prepSteps.reserve(prepareStepVarList.size());
    for(const auto& prep : qAsConst(prepareStepVarList))
        this->addPrepareStep(PrepareStep(SerializationHelper::parsePrepareStep(prep)));

    this->setTimeStamp(varMap.value("time stamp").toDouble());
}

/*!
 * Stores all member variables in a QVariant.
 */
QVariant AcquisitionSetup::View::toVariant() const
{
    QVariantMap ret;

    QVariantList prepareStepVarList;
    prepareStepVarList.reserve(int(_prepareSteps.size()));
    for(const auto& prep : _prepareSteps)
        prepareStepVarList.append(prep->toVariant());

    ret.insert("time stamp", _timeStamp);
    ret.insert("prepare steps", prepareStepVarList);

    return ret;
}

/*!
 * private helper; non-const version of `prepareStep(int, bool) const`
 */
AcquisitionSetup::PrepareStep& AcquisitionSetup::View::prepareStep(int prepareStepType,
                                                                   bool searchFromBack)
{
    return const_cast<PrepareStep&>(
           const_cast<const AcquisitionSetup::View&>(*this).prepareStep(prepareStepType,
                                                                        searchFromBack));
}

/*!
 * Creates an AcquisitionSetup with \a nbViews views that uses the CTSystem \a system.
 *
 * If \a nbViews = 0, make sure to explicitely set the desired number of views with setNbViews()
 * and adjust the views for the required purpose (either individually or by use of a preparation
 * protocol, see applyPreparationProtocol()) before using the setup. Alternatively all views can be
 * added individually with addView().
 */
AcquisitionSetup::AcquisitionSetup(const CTSystem& system, uint nbViews)
{
    this->resetSystem(system);
    this->setNbViews(nbViews);
}

/*!
 * Creates an AcquisitionSetup with \a nbViews views that uses the CTSystem \a system.
 */
AcquisitionSetup::AcquisitionSetup(CTSystem&& system, uint nbViews)
{
    this->resetSystem(std::move(system));
    this->setNbViews(nbViews);
}

/*!
 * Creates an AcquisitionSetup with \a nbViews views that uses the CTSystem \a system.
 */
AcquisitionSetup::AcquisitionSetup(std::unique_ptr<CTSystem> system, uint nbViews)
{
    if(system)
        this->resetSystem(std::move(*system));
    this->setNbViews(nbViews);
}

/*!
 * Creates an AcquisitionSetup with \a nbViews views that uses the CTSystem \a system.
 */
AcquisitionSetup::AcquisitionSetup(std::unique_ptr<SimpleCTSystem> system, uint nbViews)
    : _system(std::move(system))
{
    this->setNbViews(nbViews);
}

/*!
 * Creates an AcquisitionSetup with \a nbViews views without a CTSystem.
 *
 * Note that a CTSystem must be set explicitely with resetSystem() before the setup can be used.
 */
AcquisitionSetup::AcquisitionSetup(uint nbViews)
{
    this->setNbViews(nbViews);
}

/*!
 * Creates a copy of \a other. This uses CTSystem::clone() to create a deep copy of the CTSystem
 * member variable.
 */
AcquisitionSetup::AcquisitionSetup(const AcquisitionSetup& other)
    : SerializationInterface(other)
    , _system(other._system ? static_cast<SimpleCTSystem*>(other._system->clone()) : nullptr)
    , _views(other._views)
{
}

/*!
 * Assigns the content of \a other to this instance. This uses CTSystem::clone() to create a deep
 * copy of the CTSystem member variable.
 */
AcquisitionSetup& AcquisitionSetup::operator=(const AcquisitionSetup& other)
{
    _system.reset(other._system ? static_cast<SimpleCTSystem*>(other._system->clone()) : nullptr);
    _views = other._views;
    return *this;
}

/*!
 * Adds the View \a view to this setup.
 */
void AcquisitionSetup::addView(AcquisitionSetup::View view) { _views.push_back(std::move(view)); }

/*!
 * Applies the prepration protocol \a preparation to this setup. This means that the prepare steps
 * created by AbstractPreparationProtocol::prepareSteps() are appended to all views in this setup.
 * The consequences of this aspect are, in particular, that application of multiple preparation
 * protocols is cumulative. When this is not desired, consider removing all prepare steps with
 * removeAllPrepareSteps() before applying a new preparation protocol.
 *
 * Note that changing the number of views afterwards does not take into account this application of
 * \a preparation. Consequently, all views that are added later on will not contain the preparation
 * steps from \a preparation.
 *
 * \sa setNbViews().
 */
void AcquisitionSetup::applyPreparationProtocol(const AbstractPreparationProtocol& preparation)
{
    if(this->nbViews() == 0)
        qWarning() << "AcquisitionSetup::applyPreparationProtocol: trying to apply protocol to "
                      "setup with number of views = 0. This has no effect!";

    for(uint view = 0, nbViews = this->nbViews(); view < nbViews; ++view)
    {
        auto prepareSteps = preparation.prepareSteps(view, *this);
        for(auto& step : prepareSteps)
            _views[view].addPrepareStep(std::move(step));
    }

    qDebug() << "AcquisitionSetup --- addPreparationProtocol\n"
             << "- nbViews: " << _views.size();
}

/*!
 * Prepares the system of this setup for the view \a viewNb.
 *
 * This applies all prepare step queued in the corresponding View. Steps are applied in the order
 * they have been added to the View object.
 *
 * Use this method if you want to inspect the system configuration for a certain view in the setup.
 *
 * Example: reading the tube voltage of an XrayTube component for all views
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * // note that a GenericCarmCT has a default X-ray tube voltage of 100 kV
 *
 * AcquisitionSetup setup(system, 100);
 * // ... arbitrary view configuration comes here
 *
 * // read out the voltages for all views:
 * const auto tube = static_cast<XrayTube*>(setup.system()->source()); // we remember a pointer to the XrayTube in the system
 * XYDataSeries voltages;
 * for(uint v = 0; v < setup.nbViews(); ++v)
 * {
 *     setup.prepareView(v);                    // this prepares the system for view "v"
 *     voltages.append(v, tube->tubeVoltage()); // this reads the tube voltage and adds it to our data series
 * }
 *
 * // If you have included the 'ctl_qtgui.pri' module (or submodule 'gui_widgets_charts.pri') in the project,
 * // you can use the LineSeriesView class to visualize the read-out voltages.
 * gui::LineSeriesView::plot(voltages, "View index", "Tube voltage [kV]");
 * \endcode
 */
void AcquisitionSetup::prepareView(uint viewNb)
{
    if(!_system)
        return;

    for(const auto& step : _views[viewNb].prepareSteps())
        step->prepare(*_system);
}

/*!
 * Removes all prepare steps from all views of this setup. This leaves the setup with the same
 * number of views as it had beforehand. If \a keepTimeStamps is \c true, the time stamps from
 * the previous views are preserved. Otherwise, views are created with default time stamps.
 *
 * This method can be used, for instance, to re-use the same setup with a different trajectory
 * protocol applied. In the following example, we use the same setup with a GenericCarmCT once for
 * an acquisition with a short-scan trajectory and then again for a wobble trajectory:
 * \code
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 *
 * AcquisitionSetup setup(system, 100);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(500.0));
 *
 * // we do someting with the setup, e.g. visualize the setup or use it to create projections
 * gui::AcquisitionSetupView::plot(setup); // note: this requires 'gui_widgets_3d.pri' submodule
 * auto volume = VoxelVolume<float>::cube(100, 1.0f, 0.02f);
 * auto projector = makeProjector<OCL::RayCasterProjector>();
 * projector->configure(setup);
 * auto projections = projector->project(volume);
 *
 * // we now change the geometry, by ...
 * //... first, removing all prepare steps (previously added by the short scan protocol)
 * setup.removeAllPrepareSteps();
 * //... then, applying the protocol for the new trajectory
 * setup.applyPreparationProtocol(protocols::WobbleTrajectory(200.0_deg, 500.0));
 *
 * // now, we can repeat the steps that do something with our setup (which now represents a wobble scan)
 * gui::AcquisitionSetupView::plot(setup); // note: this requires 'gui_widgets_3d.pri' submodule
 * projector->configure(setup);
 * projections = projector->project(volume);
 * \endcode
 */
void AcquisitionSetup::removeAllPrepareSteps(bool keepTimeStamps)
{
    if(keepTimeStamps)
    {
        for(auto& view : _views)
            view.clearPrepareSteps();
    }
    else
    {
        uint prevNbViews = nbViews();
        _views.clear();
        setNbViews(prevNbViews);
    }
}

/*!
 * Removes all views from the setup. Same as setNbViews(0).
 */
void AcquisitionSetup::removeAllViews() { setNbViews(0); }

/*!
 * Sets the system of this setup to \a system. This creates a deep copy of \a system using
 * CTSystem::clone(). The previous system is deleted.
 *
 * \a system must be convertible to a SimpleCTSystem. Otherwise the system will be set to nullptr.
 */
bool AcquisitionSetup::resetSystem(const CTSystem& system)
{
    bool ok;
    auto clonedSystem = static_cast<SimpleCTSystem*>(
                SimpleCTSystem::fromCTSystem(system, &ok).clone());

    if(ok)
        _system.reset(clonedSystem);
    else
        _system = nullptr;

    return ok;
}

/*!
 * Sets the system of this setup to \a system. This moves \a system to this instance. The previous
 * system is deleted.
 *
 * \a system must be convertible to a SimpleCTSystem. Otherwise the system will be set to nullptr.
 */
bool AcquisitionSetup::resetSystem(CTSystem&& system)
{
    bool ok;
    auto clonedSystem = static_cast<SimpleCTSystem*>(
        SimpleCTSystem::fromCTSystem(std::move(system), &ok).clone());

    if(ok)
        _system.reset(clonedSystem);
    else
        _system = nullptr;

    return ok;
}

/*!
 * Returns true if this setup is valid. To be valid, the following conditions must be fulfilled:
 * \li the system must be set properly (no nullptr),
 * \li the number of views must be non-zero,
 * \li all prepare steps in all views must be applicable to the system.
 */
bool AcquisitionSetup::isValid() const
{
    if(!_system)
        return false;

    if(nbViews() == 0)
        return false;

    for(const auto& vi : _views)
        for(const auto& prep : vi.prepareSteps())
            if(!prep->isApplicableTo(*_system))
                return false;

    return true;
}

/*!
 * Returns the number of views in this setup.
 */
uint AcquisitionSetup::nbViews() const { return static_cast<uint>(_views.size()); }

/*!
 * Sets the number of views in this setup to \a nbViews. Depending on the current number of views,
 * this has either of the following effects:
 * \li If \a nbViews is less than the current number of views, all excess views are removed
 * \li If \a nbViews is larger than the current number of views, empty views are appended to this
 * setup to reach the requested number of views. The time stamps of the newly created views will
 * continue from the time stamp of the last original view with the time increment between the last
 * two views (if the number of views was less than two, the time increment will be 1.0).
 */
void AcquisitionSetup::setNbViews(uint nbViews)
{
    if(nbViews <= this->nbViews())
    {
        _views.resize(nbViews);
    }
    else
    {
        uint prevNbViews = this->nbViews();
        double lastTimestamp = prevNbViews ? _views.back().timeStamp() : -1.0;
        double timeIncrement = (prevNbViews > 1)
                ? view(prevNbViews-1).timeStamp() - view(prevNbViews-2).timeStamp()
                : 1.0;

        _views.reserve(nbViews);
        for(uint v = 0, reqNewViews = nbViews-prevNbViews; v < reqNewViews; ++v)
            _views.emplace_back(lastTimestamp + (v+1)*timeIncrement);
    }
}

/*!
 * Returns a pointer to the system in this setup.
 */
SimpleCTSystem* AcquisitionSetup::system()
{
    if(_system == nullptr)
        qWarning("No CT system has been set for the AcquisitionSetup.");
    return _system.get();
}

/*!
 * Returns a pointer to the (constant) system in this setup.
 */
const SimpleCTSystem* AcquisitionSetup::system() const
{
    if(_system == nullptr)
        qWarning("No CT system has been set for the AcquisitionSetup.");
    return _system.get();
}

/*!
 * Returns a reference to the View \a viewNb of this setup.
 *
 * This does not perform boundary checks.
 */
AcquisitionSetup::View& AcquisitionSetup::view(uint viewNb) { return _views[viewNb]; }

/*!
 * Returns a constant reference to the View \a viewNb of this setup.
 *
 * This does not perform boundary checks.
 */
const AcquisitionSetup::View& AcquisitionSetup::view(uint viewNb) const { return _views[viewNb]; }

/*!
 * Returns a reference to the vector of views of this setup.
 */
std::vector<AcquisitionSetup::View>& AcquisitionSetup::views() { return _views; }

/*!
 * Returns a constant reference to the vector of views of this setup.
 */
const std::vector<AcquisitionSetup::View>& AcquisitionSetup::views() const { return _views; }

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void AcquisitionSetup::fromVariant(const QVariant &variant)
{
    auto varMap = variant.toMap();

    CTSystem system;
    system.fromVariant(varMap.value("CT system"));
    this->resetSystem(std::move(system));

    QVariantList viewVarList = varMap.value("views").toList();
    for(const auto& v : qAsConst(viewVarList))
    {
        AcquisitionSetup::View view;
        view.fromVariant(v);
        this->addView(std::move(view));
    }
}

/*!
 * Stores all member variables in a QVariant.
 */
QVariant AcquisitionSetup::toVariant() const
{
    QVariantMap ret;

    QVariantList viewVarList;
    viewVarList.reserve(int(_views.size()));
    for(const auto& v : _views)
        viewVarList.append(v.toVariant());

    ret.insert("CT system", _system ? _system->toVariant() : QVariant());
    ret.insert("views", viewVarList);

    return ret;
}

/*!
 * \fn AcquisitionSetup::AcquisitionSetup(AcquisitionSetup&& other)
 *
 * Default move constructor.
 */

/*!
 * \fn AcquisitionSetup& AcquisitionSetup::operator=(AcquisitionSetup&& other)
 *
 * Default move assignment operator.
 */

} // namespace CTL
