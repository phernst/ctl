#include "dynamicprojectorextension.h"
#include "acquisition/acquisitionsetup.h"
#include "components/abstractdetector.h"
#include "img/abstractdynamicvolumedata.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(DynamicProjectorExtension)

void DynamicProjectorExtension::configure(const AcquisitionSetup& setup)
{
    _setup = setup;

    ProjectorExtension::configure(setup);
}

/*!
 * Computes the projections of \a volume using the acquisition setup set previously with
 * configure().
 *
 * This extension enables support for volume data that change over time (i.e. from view to view).
 * To be more specific, volume data in \a volume is updated to the next time step in advance of
 * processing each view. Supposing the passed volume data (i.e. \a volume) is a dynamic volume, the
 * internal workflow is as follows:
 *
 * For each view in the setup:
 * 1. Set the time for \a volume to the time stamp encoded in the setup for the current view.
 * This updates the volume's contents (see AbstractDynamicVolumeData::setTime()).
 * 2. Prepare the current view.
 * 3. Configure the nested projector with an AcquisitionSetup containing the current system for
 * one view (the current one).
 * 4. Compute the projection and append the result to the full set of projections.
 *
 * If \a volume is not a dynamic volume (see AbstractDynamicVolumeData), this extension is skipped
 * and the projection operation is delegated to the nested projector instead.
 */
ProjectionData DynamicProjectorExtension::project(const VolumeData& volume)
{
    auto dynamicVolPtr = dynamic_cast<const AbstractDynamicVolumeData*>(&volume);
    if(!dynamicVolPtr)
    {
        ProjectorExtension::configure(_setup);
        return ProjectorExtension::project(volume);
    }

    // notifier()->disconnect();

    auto volCopy = static_cast<AbstractDynamicVolumeData*>(dynamicVolPtr->clone());

    ProjectionData ret(_setup.system()->detector()->viewDimensions());

    for(auto view = 0u, nbViews = _setup.nbViews(); view < nbViews; ++view)
    {
        volCopy->setTime(_setup.view(view).timeStamp());
        _setup.prepareView(view);

        ProjectorExtension::configure({ *_setup.system(), 1 });
        ret.append(ProjectorExtension::project(*volCopy).view(0));

        // emit notifier()->projectionFinished(static_cast<int>(view));
    }

    return ret;
}

ProjectionData DynamicProjectorExtension::projectComposite(const CompositeVolume& volume)
{
    return AbstractProjector::projectComposite(volume);
}

// Use SerializationInterface::toVariant() documentation.
QVariant DynamicProjectorExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "DynamicProjectorExtension");

    return ret;
}

} // namespace CTL
