#ifndef RAYCASTERADAPTER_H
#define RAYCASTERADAPTER_H

#include "abstractprojector.h"
#include "abstractprojectorconfig.h"
#include "mat/mat.h"
#include "raycaster.h"

namespace CTL {
namespace OCL {

class RayCasterAdapter : public AbstractProjector
{
public:
    class Config : public AbstractProjectorConfig
    {
    public:
        float increment_mm{ 0.1f };

        AbstractProjectorConfig* clone() const override;
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

#endif // RAYCASTERADAPTER_H
