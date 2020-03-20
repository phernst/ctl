#ifndef CTL_RAYCASTERADAPTER_H
#define CTL_RAYCASTERADAPTER_H

#include "abstractprojector.h"
#include "mat/mat.h"
#include "raycaster.h"

namespace CTL {
namespace OCL {

class RayCasterAdapter : public AbstractProjector
{
public:
    class Config
    {
    public:
        float increment_mm{ 0.1f };
    };

    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;

    void applyRayCasterConfig(const Config& rcConfig);

protected:
    RayCaster _rayCaster;

    SingleViewData::Dimensions _viewDim;
    QVector<ProjectionMatrix> _pMatsVectorized;
};

} // namespace OCL
} // namespace CTL

#endif // CTL_RAYCASTERADAPTER_H
