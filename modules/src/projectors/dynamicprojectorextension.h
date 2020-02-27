#ifndef DYNAMICPROJECTOREXTENSION_H
#define DYNAMICPROJECTOREXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"
#include "img/abstractdynamicvolumedata.h"

namespace CTL {

class DynamicProjectorExtension : public ProjectorExtension
{
    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;
    public: ProjectionData project(const VolumeData& volume) override;
    public: ProjectionData projectComposite(const CompositeVolume &volume) override;

public:
    using ProjectorExtension::ProjectorExtension;

private:
    AcquisitionSetup _setup; //!< used acquisition setup

};

} // namespace CTL

#endif // DYNAMICPROJECTOREXTENSION_H
