#ifndef DYNAMICPROJECTOR_H
#define DYNAMICPROJECTOR_H

#include "abstractprojector.h"
#include "acquisition/acquisitionsetup.h"
#include "img/abstractdynamicvoxelvolume.h"

namespace CTL {

class DynamicProjector : public AbstractProjector
{
    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;
    public: ProjectionData project(const VolumeData& volume) override;

public:
    explicit DynamicProjector(AbstractProjector* projector);
    explicit DynamicProjector(std::unique_ptr<AbstractProjector> projector);

    // using _projector view by view (reconfigure _projector for each view)
    ProjectionData project(AbstractDynamicVoxelVolume& volume);

private:
    std::unique_ptr<AbstractProjector> _projector; //!< used static projector
    AcquisitionSetup _setup; //!< used acquisition setup
};

} // namespace CTL

#endif // DYNAMICPROJECTOR_H
