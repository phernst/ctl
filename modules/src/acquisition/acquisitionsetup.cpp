#include <QDebug>

#include "acquisitionsetup.h"

namespace CTL {

AcquisitionSetup::View::View(double time)
    : timeStamp(time)
{
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
            _views[view].prepareSteps.push_back(std::move(step));
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

    for(const auto& step : _views[viewNb].prepareSteps)
        step->prepare(_system.get());
}

void AcquisitionSetup::removeAllPrepareSteps()
{
    for(auto& view : _views)
        view.prepareSteps.clear();
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
        for(const auto& prep : vi.prepareSteps)
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

} // namespace CTL
