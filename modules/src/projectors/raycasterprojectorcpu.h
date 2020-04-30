#ifndef CTL_RAYCASTERPROJECTORCPU_H
#define CTL_RAYCASTERPROJECTORCPU_H

#include "abstractprojector.h"
#include "acquisition/viewgeometry.h"

namespace CTL {

class RayCasterProjectorCPU : public AbstractProjector
{
    CTL_TYPE_ID(10)

    class Settings
    {
    public:
        uint raysPerPixel[2] = { 1, 1 }; //!< number of ray per pixel in channel (x) and row (y) direction
        float raySampling = 0.3f; //!< fraction of voxel size that is used to increment position on ray
        bool interpolate = true; //!< enables interpolation of voxel value (attenuation) during ray casting
    };

public:
    // AbstractProjector interface
    void configure(const AcquisitionSetup &setup) override;
    ProjectionData project(const VolumeData &volume) override;

    Settings& settings();

    // SerializationInterface interface
    QVariant toVariant() const override;
    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    Settings _settings; //!< settings of the projector
    SingleViewData::Dimensions _viewDim; //!< dimensions of a single view
    FullGeometry _pMats; //!< full set of projection matrices for all views and modules

    SingleViewData computeView(const VolumeData& volume,
                               const mat::Matrix<3,1>& volumeCorner,
                               uint view) const;
};

} // namespace CTL

#endif // CTL_RAYCASTERPROJECTORCPU_H
