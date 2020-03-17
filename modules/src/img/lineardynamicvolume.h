#ifndef LINEARDYNAMICVOLUME_H
#define LINEARDYNAMICVOLUME_H

#include "abstractdynamicvolumedata.h"
#include <cmath>
/*
 * NOTE: This is header only.
 */

namespace CTL {

/*!
 * \class LinearDynamicVolume
 *
 * \brief The LinearDynamicVolume class is an implementation of AbstractDynamicVolumeData with a
 * linear relation of the attenuation values of each voxel.
 *
 * The following example code shows how to create a ball whose attenuation values increase linearly
 * over time as well as a cubic volume holding linearly decreasing values:
 * \code
 * // create a ball phantom with attenuation values increasing by 0.1/mm each millisecond
 * // attenuation values start at 0.0/mm at time point 0 ms
 * LinearDynamicVolume dynamicBall(VoxelVolume<float>::ball(30.0f, 1.0f, 0.1f),
 *                                 VoxelVolume<float>::ball(30.0f, 1.0f, 0.0f));
 *
 * // create a cubic phantom with attenuation values decreasing by 0.05/mm each millisecond
 * // attenuation values start at 1.0/mm at time point 0 ms
 * LinearDynamicVolume dynamicCube(VoxelVolume<float>::cube(100, 1.0f, -0.05f),
 *                                 VoxelVolume<float>::cube(100, 1.0f, 1.0f));
 * \endcode
 */
class LinearDynamicVolume : public AbstractDynamicVolumeData
{
    // abstract interface
    protected: void updateVolume() override;
    public: SpectralVolumeData* clone() const override;

public:
    LinearDynamicVolume(float slope,
                        float offset,
                        const VoxelVolume<float>::Dimensions& nbVoxel,
                        const VoxelVolume<float>::VoxelSize& voxelSize);
    LinearDynamicVolume(VoxelVolume<float> slope,
                        VoxelVolume<float> offset,
                        const VoxelVolume<float>::VoxelSize& voxelSize);
    LinearDynamicVolume(VoxelVolume<float> slope,
                        VoxelVolume<float> offset);

private:
    VoxelVolume<float> _offset;
    VoxelVolume<float> _slope;
};

} // namespace CTL

#endif // LINEARDYNAMICVOLUME_H
