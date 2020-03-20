#ifndef CTL_RAYCASTERPROJECTOR_H
#define CTL_RAYCASTERPROJECTOR_H

#include "abstractprojector.h"
#include "acquisition/geometryencoder.h"

namespace CTL {
namespace OCL {

/*!
 * \class RayCasterProjector
 *
 * \brief The RayCasterProjector class is an implementation of a simple OpenCL-based ray casting
 * projector that uses a constant step-width algorithm.
 *
 * This class is a direct implementation of AbstractProjector that has no external dependencies
 * (except for OpenCL). This projector is based on a ray casting routine with constant-step width
 * interpolation on Image3D objects of OpenCL devices.
 */
class RayCasterProjector : public AbstractProjector
{
    CTL_TYPE_ID(1)

public:    
    RayCasterProjector();

    class Settings
    {
    public:
        std::vector<uint> deviceIDs; //!< used device IDs in the OpenCLConfig device list (if empty: use all)
        uint raysPerPixel[2] = { 1, 1 }; //!< number of ray per pixel in channel (x) and row (y) direction
        float raySampling = 0.3f; //!< fraction of voxel size that is used to increment position on ray
        uint volumeUpSampling = 1; //!< factor that increases the number of voxels in each dimension
        bool interpolate = true; //!< enables interpolation of voxel value (attenuation) during ray casting

        static Settings optimizedFor(const VolumeData& volume, const AbstractDetector &detector);
    };

    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;

    // SerializationInterface interface
    QVariant toVariant() const override;
    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

    Settings& settings();

private:
    void initOpenCL();
    void prepareOpenCLDeviceList();

    Settings _settings; //!< settings of the projector
    std::string _oclProgramName; //!< OCL program name (depends on if interpolation is enabled)
    SingleViewData::Dimensions _viewDim; //!< dimensions of a single view
    uint _volDim[3]; //!< cache for volume dimensions
    FullGeometry _pMats; //!< full set of projection matrices for all views and modules
};

/*!
 * \class RayCasterProjector::Config
 * \brief Holds the configuration for RayCasterProjector objects.
 */

} // namespace OCL
} // namespace CTL

#endif // CTL_RAYCASTERPROJECTOR_H
