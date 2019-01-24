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

    void configure(const AcquisitionSetup& setup,
                   const AbstractProjectorConfig& config) override;
    ProjectionData project(const VolumeData& volume) override;

protected:
    RayCaster _rayCaster;

    QVector<ProjectionMatrix> _pMatsVectorized;
    SingleViewData::Dimensions _viewDim;

    void applyRayCasterConfig(const Config& rcConfig);
};

} // namespace OCL
} // namespace CTL

#endif // RAYCASTERADAPTER_H
