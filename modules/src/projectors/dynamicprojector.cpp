#include "dynamicprojector.h"
#include "abstractprojectorconfig.h"
#include "acquisition/acquisitionsetup.h"
#include "components/abstractdetector.h"

namespace CTL {

static SingleViewData::Dimensions extractViewDimensions(const AbstractDetector& detector);

DynamicProjector::DynamicProjector(AbstractProjector* projector)
    : _projector(projector)
{
}

DynamicProjector::DynamicProjector(std::unique_ptr<AbstractProjector> projector)
    : _projector(std::move(projector))
{
}

void DynamicProjector::configure(const AcquisitionSetup &setup,
                                 const AbstractProjectorConfig& config)
{
    _setup = setup;
    _projectorConfig.reset(config.clone());
}

ProjectionData DynamicProjector::project(const VolumeData &volume)
{
    auto notfierConnection =
            QObject::connect(_projector->notifier(), &ProjectorNotifier::projectionFinished,
                             this->notifier(), &ProjectorNotifier::projectionFinished);

    _projector->configure(_setup, *_projectorConfig);
    auto ret = _projector->project(volume);

    QObject::disconnect(notfierConnection);
    return ret;
}

ProjectionData DynamicProjector::project(AbstractDynamicVoxelVolume* volume)
{
    ProjectionData ret(extractViewDimensions(*_setup.system()->detector()));

    for(uint view = 0, nbViews = _setup.nbViews(); view < nbViews; ++view)
    {
        volume->setTime(_setup.view(view).timeStamp);
        _setup.prepareView(view);

        _projector->configure({ *_setup.system(), 1 }, *_projectorConfig);
        ret.append(_projector->project(*volume).view(0));

        emit notifier()->projectionFinished(view);
    }

    return ret;
}

SingleViewData::Dimensions extractViewDimensions(const AbstractDetector& detector)
{
    SingleViewData::Dimensions ret;
    auto detectorPixels = detector.nbPixelPerModule();
    ret.nbRows = detectorPixels.height();
    ret.nbChannels = detectorPixels.width();
    ret.nbModules = detector.nbDetectorModules();
    return ret;
}

} // namespace CTL
