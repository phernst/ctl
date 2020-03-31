#include "lineardynamicvolume.h"

namespace CTL {

/*!
 * Constructs a LinearDynamicVolume with linear relation for the attenuation coefficients in each
 * voxel \f$\mu(x,y,z)\f$ specified by \a slope and \a offset, corresponding to:
 *
 * \f$ \mu(x,y,z) = t \cdot slope(x,y,z) + offset(x,y,z) \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 *
 * The voxel size is set to \a voxelSize.
 * Note that the number of voxels in \a slope and \a offset must be equal; throws an exception
 * otherwise.
 */
LinearDynamicVolume::LinearDynamicVolume(VoxelVolume<float> slope,
                                         VoxelVolume<float> offset,
                                         const VoxelVolume<float>::VoxelSize& voxelSize)
    : AbstractDynamicVolumeData( SpectralVolumeData( VoxelVolume<float>(slope.dimensions(), voxelSize) ) )
    , _lag(std::move(offset))
    , _slope(std::move(slope))
{
    if(_lag.dimensions() != _slope.dimensions())
        throw std::domain_error("LinearDynamicVolume: Passed offset and slope volumes have different sizes!");

    setTime(0.0); // initial time point 0ms
}

/*!
 * Constructs a LinearDynamicVolume with linear relation for the attenuation coefficients in each
 * voxel \f$\mu(x,y,z)\f$ specified by \a slope and \a offset, corresponding to:
 *
 * \f$ \mu(x,y,z) = t \cdot slope(x,y,z) + offset(x,y,z) \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 *
 * The voxel size is taken from \a slope.
 * Note that both the number of voxels and the voxel size in \a slope and \a offset must be equal;
 * throws an exception otherwise.
 */
LinearDynamicVolume::LinearDynamicVolume(VoxelVolume<float> slope, VoxelVolume<float> offset)
    : LinearDynamicVolume(std::move(slope), std::move(offset), slope.voxelSize())
{
    if(_lag.voxelSize() != _slope.voxelSize())
        throw std::domain_error("LinearDynamicVolume: Passed offset and slope volumes have different sizes!");
}

/*!
 * Constructs a LinearDynamicVolume with \a \nbVoxel voxels in each dimension (size: \a voxelSize)
 * and assigns an identical linear relation for the attenuation coefficients of all voxels
 * \f$\mu(x,y,z)\f$ specified by \a slope and \a offset, corresponding to:
 *
 * \f$ \mu(x,y,z) = t \cdot slope + offset \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 */
LinearDynamicVolume::LinearDynamicVolume(float slope, float offset,
                                         const Dimensions& nbVoxel, const VoxelSize& voxelSize)
    : AbstractDynamicVolumeData( SpectralVolumeData( VoxelVolume<float>(nbVoxel, voxelSize) ) )
    , _lag(nbVoxel, voxelSize)
    , _slope(nbVoxel, voxelSize)
{
    _lag.fill(offset);
    _slope.fill(slope);

    setTime(0.0); // initial time point 0ms
}

/*!
 * Constructs a LinearDynamicVolume volume with voxels of isotropic dimensions \a voxelSize (in mm)
 * All voxels inside a ball of radius \a radius (in mm) around the center of the volume will follow
 * a linear relation for their attenuation values of:
 *
 * \f$ \mu(x,y,z) = t \cdot slope + offset \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 *
 * The voxels surrounding the ball are filled with zeros.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * each dimension.
 */
LinearDynamicVolume LinearDynamicVolume::ball(float radius, float voxelSize,
                                              float slope, float offset)
{

    return LinearDynamicVolume(VoxelVolume<float>::ball(radius, voxelSize, slope),
                               VoxelVolume<float>::ball(radius, voxelSize, offset));
}

/*!
 * Constructs a cubic LinearDynamicVolume volume with \a nbVoxel x \a nbVoxel x \a nbVoxel voxels
 * (voxel dimension: \a voxelSize x \a voxelSize x \a voxelSize). All voxels will follow a linear
 * relation for their attenuation values of:
 *
 * \f$ \mu(x,y,z) = t \cdot slope + offset \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 */
LinearDynamicVolume LinearDynamicVolume::cube(uint nbVoxel, float voxelSize,
                                              float slope, float offset)
{
    return LinearDynamicVolume(VoxelVolume<float>::cube(nbVoxel, voxelSize, slope),
                               VoxelVolume<float>::cube(nbVoxel, voxelSize, offset));
}

/*!
 * Constructs a LinearDynamicVolume volume with voxels of isotropic dimensions \a voxelSize (in mm)
 * All voxels inside a cylinder of radius \a radius (in mm) and height \a height (in mm) aligned
 * with the *x*-axis  will follow
 * a linear relation for their attenuation values of:
 *
 * \f$ \mu(x,y,z) = t \cdot slope + offset \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 *
 * The voxels surrounding the ball are filled with zeros.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * *y*- and *z*-dimension and \f$ \left\lceil height/voxelSize\right\rceil \f$ in *x*-direction.
 */
LinearDynamicVolume LinearDynamicVolume::cylinderX(float radius, float height, float voxelSize,
                                                   float slope, float offset)
{
    return LinearDynamicVolume(VoxelVolume<float>::cylinderX(radius, height, voxelSize, slope),
                               VoxelVolume<float>::cylinderX(radius, height, voxelSize, offset));
}

/*!
 * Constructs a LinearDynamicVolume volume with voxels of isotropic dimensions \a voxelSize (in mm)
 * All voxels inside a cylinder of radius \a radius (in mm) and height \a height (in mm) aligned
 * with the *y*-axis  will follow
 * a linear relation for their attenuation values of:
 *
 * \f$ \mu(x,y,z) = t \cdot slope + offset \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 *
 * The voxels surrounding the ball are filled with zeros.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * *x*- and *z*-dimension and \f$ \left\lceil height/voxelSize\right\rceil \f$ in *y*-direction.
 */
LinearDynamicVolume LinearDynamicVolume::cylinderY(float radius, float height, float voxelSize,
                                                   float slope, float offset)
{
    return LinearDynamicVolume(VoxelVolume<float>::cylinderY(radius, height, voxelSize, slope),
                               VoxelVolume<float>::cylinderY(radius, height, voxelSize, offset));
}

/*!
 * Constructs a LinearDynamicVolume volume with voxels of isotropic dimensions \a voxelSize (in mm)
 * All voxels inside a cylinder of radius \a radius (in mm) and height \a height (in mm) aligned
 * with the *z*-axis  will follow
 * a linear relation for their attenuation values of:
 *
 * \f$ \mu(x,y,z) = t \cdot slope + offset \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 *
 * The voxels surrounding the ball are filled with zeros.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * *x*- and *y*-dimension and \f$ \left\lceil height/voxelSize\right\rceil \f$ in *z*-direction.
 */
LinearDynamicVolume LinearDynamicVolume::cylinderZ(float radius, float height, float voxelSize,
                                                   float slope, float offset)
{
    return LinearDynamicVolume(VoxelVolume<float>::cylinderZ(radius, height, voxelSize, slope),
                               VoxelVolume<float>::cylinderZ(radius, height, voxelSize, offset));
}

SpectralVolumeData* LinearDynamicVolume::clone() const
{
    return new LinearDynamicVolume(*this);
}

/*!
 * Sets the voxels to the values given by the linear relation:
 *
 * \f$ \mu(x,y,z) = t \cdot slope(x,y,z) + offset(x,y,z) \f$, where \f$t\f$ denotes the time point
 * set via setTime() in milliseconds.
 */
void LinearDynamicVolume::updateVolume()
{
    auto updatedVol = _slope * float(time()) + _lag;
    setData(std::move(updatedVol.data()));
}

} // namespace CTL
