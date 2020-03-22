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

SpectralVolumeData* LinearDynamicVolume::clone() const
{
    return new LinearDynamicVolume(*this);
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
