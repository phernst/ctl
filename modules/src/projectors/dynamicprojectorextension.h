#ifndef DYNAMICPROJECTOREXTENSION_H
#define DYNAMICPROJECTOREXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"

namespace CTL {

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

#endif // DYNAMICPROJECTOREXTENSION_H
