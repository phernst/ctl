#include <QDebug>

#include "acquisitionsetup.h"

namespace CTL {

AcquisitionSetup::View::View(double time)
    : _timeStamp(time)
{
}

void AcquisitionSetup::View::setTimeStamp(double timeStamp) { _timeStamp = timeStamp; }

void AcquisitionSetup::View::addPrepareStep(PrepareStep step)
{
    if(step)
        _prepareSteps.push_back(std::move(step));
    else
        qWarning() << "AcquisitionSetup::View::addPrepareStep(): Prepare step not added! "
                      "Reason: tried to add 'nullptr'.";
}

double AcquisitionSetup::View::timeStamp() const { return _timeStamp; }

const std::vector<AcquisitionSetup::PrepareStep>& AcquisitionSetup::View::prepareSteps() const
{
    return _prepareSteps;
}

void AcquisitionSetup::View::clearPrepareSteps() { _prepareSteps.clear(); }

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

AcquisitionSetup::AcquisitionSetup(const CTsystem& system, uint nbViews)
{
    this->resetSystem(system);
    this->setNbViews(nbViews);
}

AcquisitionSetup::AcquisitionSetup(CTsystem&& system, uint nbViews)
{
    this->resetSystem(std::move(system));
    this->setNbViews(nbViews);
}

AcquisitionSetup::AcquisitionSetup(const AcquisitionSetup& other)
    : _system(other._system ? static_cast<SimpleCTsystem*>(other._system->clone()) : nullptr)
    , _views(other._views)
{
}

AcquisitionSetup& AcquisitionSetup::operator=(const AcquisitionSetup& other)
{
    _system.reset(other._system ? static_cast<SimpleCTsystem*>(other._system->clone()) : nullptr);
    _views = other._views;
    return *this;
}

void AcquisitionSetup::addView(AcquisitionSetup::View view) { _views.push_back(std::move(view)); }

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

void AcquisitionSetup::prepareView(uint viewNb)
{
    if(!_system)
        return;

    for(const auto& step : _views[viewNb].prepareSteps())
        step->prepare(_system.get());
}

void AcquisitionSetup::removeAllPrepareSteps()
{
    for(auto& view : _views)
        view.clearPrepareSteps();
}

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

uint AcquisitionSetup::nbViews() const { return static_cast<uint>(_views.size()); }

void AcquisitionSetup::setNbViews(uint nbViews)
{
    if(nbViews <= this->nbViews())
    {
        _views.resize(nbViews);
    }
    else
    {
        _views.reserve(nbViews);
        for(uint viewTime = this->nbViews(); viewTime < nbViews; ++viewTime)
            _views.emplace_back(double(viewTime));
    }
}

SimpleCTsystem* AcquisitionSetup::system() { return _system.get(); }

const SimpleCTsystem* AcquisitionSetup::system() const { return _system.get(); }

AcquisitionSetup::View& AcquisitionSetup::view(uint viewNb) { return _views[viewNb]; }

const AcquisitionSetup::View& AcquisitionSetup::view(uint viewNb) const { return _views[viewNb]; }

std::vector<AcquisitionSetup::View>& AcquisitionSetup::views() { return _views; }

const std::vector<AcquisitionSetup::View>& AcquisitionSetup::views() const { return _views; }

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

} // namespace CTL
