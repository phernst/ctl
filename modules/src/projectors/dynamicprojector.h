#ifndef DYNAMICPROJECTOR_H
#define DYNAMICPROJECTOR_H

#include "abstractprojector.h"
#include "acquisition/acquisitionsetup.h"
#include "img/abstractdynamicvoxelvolume.h"

namespace CTL {

class DynamicProjector : public AbstractProjector
{
public:
    explicit DynamicProjector(AbstractProjector* projector);
    explicit DynamicProjector(std::unique_ptr<AbstractProjector> projector);

    void configure(const AcquisitionSetup& setup, const AbstractProjectorConfig& config) override;
    ProjectionData project(const VolumeData& volume) override;

    // using _projector view by view (reconfigure _projector for each view)
    ProjectionData project(AbstractDynamicVoxelVolume& volume);

private:
    std::unique_ptr<AbstractProjector> _projector; //!< used static projector
    std::unique_ptr<AbstractProjectorConfig> _projectorConfig; //!< config of the static projector
    AcquisitionSetup _setup; //!< used acquisition setup
};

} // namespace CTL

#endif // DYNAMICPROJECTOR_H
