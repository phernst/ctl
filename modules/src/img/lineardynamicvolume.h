#ifndef CTL_LINEARDYNAMICVOLUME_H
#define CTL_LINEARDYNAMICVOLUME_H

#include "abstractdynamicvolumedata.h"
#include <cmath>

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

    static LinearDynamicVolume ball(float radius, float voxelSize, float slope, float offset);
    static LinearDynamicVolume cube(uint nbVoxel, float voxelSize, float slope, float offset);
    static LinearDynamicVolume cylinderX(float radius, float height, float voxelSize, float slope, float offset);
    static LinearDynamicVolume cylinderY(float radius, float height, float voxelSize, float slope, float offset);
    static LinearDynamicVolume cylinderZ(float radius, float height, float voxelSize, float slope, float offset);

    XYDataSeries timeCurve(uint x, uint y, uint z, const std::vector<float>& timePoints) override;

    using AbstractDynamicVolumeData::timeCurve;

private:
    VoxelVolume<float> _lag;
    VoxelVolume<float> _slope;
};

} // namespace CTL

#endif // CTL_LINEARDYNAMICVOLUME_H
