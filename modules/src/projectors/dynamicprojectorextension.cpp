#include "dynamicprojectorextension.h"
#include "abstractprojectorconfig.h"
#include "acquisition/acquisitionsetup.h"
#include "components/abstractdetector.h"

namespace CTL {

void DynamicProjectorExtension::configure(const AcquisitionSetup& setup)
{
    _setup = setup;

    ProjectorExtension::configure(setup);
}

ProjectionData DynamicProjectorExtension::project(const VolumeData& volume)
{
    auto dynamicVolPtr = dynamic_cast<const AbstractDynamicVolumeData*>(&volume);
    if(!dynamicVolPtr)
    {
        ProjectorExtension::configure(_setup);
        return ProjectorExtension::project(volume);
    }

    notifier()->disconnect();

    auto volCopy = static_cast<AbstractDynamicVolumeData*>(dynamicVolPtr->clone());

    ProjectionData ret(_setup.system()->detector()->viewDimensions());

    for(uint view = 0, nbViews = _setup.nbViews(); view < nbViews; ++view)
    {
        volCopy->setTime(_setup.view(view).timeStamp());
        _setup.prepareView(view);

        ProjectorExtension::configure({ *_setup.system(), 1 });
        ret.append(ProjectorExtension::project(*volCopy).view(0));

        emit notifier()->projectionFinished(view);
    }

    return ret;
}

ProjectionData DynamicProjectorExtension::projectComposite(const CompositeVolume &volume)
{
    return AbstractProjector::projectComposite(volume);
}

} // namespace CTL
