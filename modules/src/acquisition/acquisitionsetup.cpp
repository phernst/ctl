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
 * Returns a constant reference to the vector of prepare steps of this instance.
 */
const std::vector<AcquisitionSetup::PrepareStep>& AcquisitionSetup::View::prepareSteps() const
{
    return _prepareSteps;
}

/*!
 * Returns a (modifiable) reference to the vector of prepare steps of this instance.
 */
std::vector<AcquisitionSetup::PrepareStep>& AcquisitionSetup::View::prepareSteps()
{
    return _prepareSteps;
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
    for(const auto& prep : prepareStepVarList)
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
 * Creates an AcquisitionSetup with \a nbViews views that uses the CTsystem \a system.
 */
AcquisitionSetup::AcquisitionSetup(const CTsystem& system, uint nbViews)
{
    this->resetSystem(system);
    this->setNbViews(nbViews);
}

/*!
 * Creates an AcquisitionSetup with \a nbViews views that uses the CTsystem \a system.
 */
AcquisitionSetup::AcquisitionSetup(CTsystem&& system, uint nbViews)
{
    this->resetSystem(std::move(system));
    this->setNbViews(nbViews);
}

/*!
 * Creates a copy of \a other. This uses CTsystem::clone() to create a deep copy of the CTsystem
 * member variable.
 */
AcquisitionSetup::AcquisitionSetup(const AcquisitionSetup& other)
    : _system(other._system ? static_cast<SimpleCTsystem*>(other._system->clone()) : nullptr)
    , _views(other._views)
{
}

/*!
 * Assigns the content of \a other to this instance. This uses CTsystem::clone() to create a deep
 * copy of the CTsystem member variable.
 */
AcquisitionSetup& AcquisitionSetup::operator=(const AcquisitionSetup& other)
{
    _system.reset(other._system ? static_cast<SimpleCTsystem*>(other._system->clone()) : nullptr);
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

    qDebug() << "AcquisitionSetup --- addPreparationProtocol";
    qDebug() << "-nbViews: " << _views.size();
}

/*!
 * Clears all views from the setup. This leaves the setup with the same number of views as it had
 * beforehand. If \a keepTimeStamps is \c true, the time stamps from the previous views are
 * preserved. Otherwise, views are created with default time stamps.
 */
void AcquisitionSetup::clearViews(bool keepTimeStamps)
{
    if(keepTimeStamps)
    {
        removeAllPrepareSteps();
    }
    else
    {
        uint prevNbViews = nbViews();
        _views.clear();
        setNbViews(prevNbViews);
    }
}

/*!
 * Prepares the system of this setup for the view \a viewNb.
 *
 * This applies all prepare step queued in the corresponding View. Steps are applied in the order
 * they have been added to the View object.
 */
void AcquisitionSetup::prepareView(uint viewNb)
{
    if(!_system)
        return;

    for(const auto& step : _views[viewNb].prepareSteps())
        step->prepare(_system.get());
}

/*!
 * Removes all prepare steps from all views of this setup. This preserves the time stamp of all
 * these View instances.
 */
void AcquisitionSetup::removeAllPrepareSteps()
{
    for(auto& view : _views)
        view.clearPrepareSteps();
}

/*!
 * Sets the system of this setup to \a system. This creates a deep copy of \a system using
 * CTsystem::clone(). The previous system is deleted.
 *
 * \a system must be convertible to a SimpleCTsystem. Otherwise the system will be set to nullptr.
 */
bool AcquisitionSetup::resetSystem(const CTsystem& system)
{
    bool ok;
    auto clonedSystem = static_cast<SimpleCTsystem*>(
                SimpleCTsystem::fromCTsystem(system, &ok).clone());

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
 * \a system must be convertible to a SimpleCTsystem. Otherwise the system will be set to nullptr.
 */
bool AcquisitionSetup::resetSystem(CTsystem&& system)
{
    bool ok;
    auto clonedSystem = static_cast<SimpleCTsystem*>(
        SimpleCTsystem::fromCTsystem(std::move(system), &ok).clone());

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
SimpleCTsystem* AcquisitionSetup::system() { return _system.get(); }

/*!
 * Returns a pointer to the (constant) system in this setup.
 */
const SimpleCTsystem* AcquisitionSetup::system() const { return _system.get(); }

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

    CTsystem system;
    system.fromVariant(varMap.value("CT system"));
    this->resetSystem(std::move(system));

    QVariantList viewVarList = varMap.value("views").toList();
    for(const auto& v : viewVarList)
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
