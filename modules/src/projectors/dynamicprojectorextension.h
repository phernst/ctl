#ifndef CTL_DYNAMICPROJECTOREXTENSION_H
#define CTL_DYNAMICPROJECTOREXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"

namespace CTL {

/*!
 * \class DynamicProjectorExtension
 *
 * \brief The DynamicProjectorExtension class is an extension for forward projectors that enables
 * processing dynamic volume data (i.e. changing from view to view).
 *
 * This extension enables support for volume data that change over time (i.e. from view to view).
 * Projections for each view are computed separately with volume data being updated to the next time
 * step in advance of each view.
 *
 * If used in combination with a static volume (i.e. not a sub-class of AbstractDynamicVolumeData),
 * this extension is skipped and the projection operation is delegated to the nested projector
 * instead.
 */
class DynamicProjectorExtension : public ProjectorExtension
{
    CTL_TYPE_ID(105)

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;
    public: ProjectionData project(const VolumeData& volume) override;

public:
    DynamicProjectorExtension() = default;
    ProjectionData projectComposite(const CompositeVolume& volume) override;

    QVariant toVariant() const override;

    using ProjectorExtension::ProjectorExtension;

private:
    AcquisitionSetup _setup; //!< used acquisition setup
};

} // namespace CTL

#endif // CTL_DYNAMICPROJECTOREXTENSION_H
