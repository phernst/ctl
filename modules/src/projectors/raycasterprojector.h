#ifndef RAYCASTERPROJECTOR_H
#define RAYCASTERPROJECTOR_H

#include "abstractprojector.h"
#include "abstractprojectorconfig.h"

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
public:
    class Config : public AbstractProjectorConfig
    {
    public:
        uint deviceID = 0; //!< used device in the OpenCLConfig device list
        uint raysPerPixel[2] = { 1, 1 }; //!< number of ray per pixel in channel (x) and row (y) direction
        float raySampling = 0.3f; //!< fraction of voxel size that is used to increment position on ray
        uint volumeUpSampling = 1; //!< factor that increases the number of voxels in each dimension
        bool interpolate = true; //!< enables interpolation of voxel value (attenuation) during ray casting

        AbstractProjectorConfig* clone() const override;
        static Config optimizedFor(const VolumeData& volume, const AbstractDetector &detector);
    };

    void configure(const AcquisitionSetup& setup,
                   const AbstractProjectorConfig& config) override;
    ProjectionData project(const VolumeData& volume) override;

private: // members
    FullGeometry _pMats; //!< full set of projection matrices for all views and modules
    SingleViewData::Dimensions _viewDim; //!< dimensions of a single view
    Config _config; //!< configuration of the projector
    std::string _oclProgramName; //!< OCL program name (depends on if interpolation is enabled)

private: // methods
    void initOpenCL();
};

// Use documentation of AbstractProjectorConfig::clone()
inline AbstractProjectorConfig* RayCasterProjector::Config::clone() const
{
    return new Config(*this);
}

/*!
 * \class RayCasterProjector::Config
 * \brief Holds the configuration for RayCasterProjector objects.
 */

} // namespace OCL
} // namespace CTL

#endif // RAYCASTERPROJECTOR_H
