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
    , _offset(std::move(offset))
    , _slope(std::move(slope))
{
    if(_offset.dimensions() != _slope.dimensions())
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
    if(_offset.voxelSize() != _slope.voxelSize())
        throw std::domain_error("LinearDynamicVolume: Passed offset and slope volumes have different sizes!");
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
void LinearDynamicVolume::updateVolume() { setData((_slope * float(time()) + _offset).data()); }

} // namespace CTL
