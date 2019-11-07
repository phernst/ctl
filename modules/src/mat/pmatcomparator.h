/******************************************************************************
** 'PMatComparator' class provides statistical evaluation of the similarity of
** two projection matrices in terms of the projection error using a certain
** geometrical setup.
**
** by Robert Frysch | Jan, 2019
** Otto von Guericke University Magdeburg
** Institute for Medical Engineering - IMT (Head: Georg Rose)
** Email: robert.frysch@ovgu.de
******************************************************************************/

#ifndef PMATCOMPARATOR_H
#define PMATCOMPARATOR_H

#include "matrix_types.h"
#include <cfloat>
#include <QSize>

typedef unsigned int uint;

namespace CTL {

template <typename T>
class VoxelVolume;

namespace mat {

class PMatComparator
{
public:
    struct Eval {
        double meanError    = 0.0;
        double minError     = DBL_MAX;
        double maxError     = 0.0;
        double stdDeviation = 0.0;
        uint64_t samples    = 0;
    };

    // discretization of test volume
    static const uint DEFAULT_NB_VOXELS = 32;

    // ctors
    PMatComparator() = default;
    PMatComparator(const QSize& nbDetectorPixels);
    PMatComparator(uint nbDetectorPixelsX, uint nbDetectorPixelsY);
    PMatComparator(const QSize& nbDetectorPixels,
                   double totVolumeSizeX, double totVolumeSizeY, double totVolumeSizeZ,
                   double accuracy = 1.0);
    PMatComparator(uint nbDetectorPixelsX, uint nbDetectorPixelsY,
                   double totVolumeSizeX, double totVolumeSizeY, double totVolumeSizeZ,
                   double accuracy = 1.0);

    // comparator function
    Eval operator() (const ProjectionMatrix& P1, const ProjectionMatrix& P2) const;
    
    // config
    bool computeStandardDeviation() const;
    bool restrictedToDetectorArea() const;
    void setRestrictionToDetectorArea(bool restrictionEnabled);
    void setComputeStandardDeviation(bool standardDeviationEnabled);

    // volume settings
    void setAccuracy(double accuracy);
    template<typename T>
    void setVolumeDefFrom(const VoxelVolume<T>& volume);
    void setVolumeGridSpacing(const Vector3x1& voxelSize);
    void setVolumeGridSpacing(double voxelSizeX, double voxelSizeY, double voxelSizeZ);
    void setVolumeOffSet(const Vector3x1& offSet);
    void setVolumeOffSet(double offSetX, double offSetY, double offSetZ);
    void setTotalVolumeSize(const Vector3x1& totVolumeSize);
    void setTotalVolumeSize(double totVolumeSizeX, double totVolumeSizeY, double totVolumeSizeZ);
    Vector3x1 totalVolumeSize() const;
    const Vector3x1& volumeGridSpacing() const;
    const Vector3x1& volumeOffset() const;

    // detector settings
    const QSize& numberDetectorPixels() const;
    void setNumberDetectorPixels(const QSize& nbPixels);
    void setNumberDetectorPixels(uint x, uint y);

private:
    typedef uint Size3D[3];
    typedef uint Size2D[2];

    // volume
    Size3D _nbVoxels{ DEFAULT_NB_VOXELS, DEFAULT_NB_VOXELS, DEFAULT_NB_VOXELS };
    Vector3x1 _voxelSize{ 8.0, 8.0, 8.0 };
    Vector3x1 _offSet{ 0.0, 0.0, 0.0 };

    // detector
    Size2D _nbPixels{ 640, 480 };
    bool _restrictToDetectorArea{ true };
    bool _enableStandardDeviation{ false };
};

template<typename T>
void PMatComparator::setVolumeDefFrom(const VoxelVolume<T>& volume)
{
    _nbVoxels[0] = volume.dimensions().x;
    _nbVoxels[1] = volume.dimensions().y;
    _nbVoxels[2] = volume.dimensions().z;

    _voxelSize(0) = volume.voxelSize().x;
    _voxelSize(1) = volume.voxelSize().y;
    _voxelSize(2) = volume.voxelSize().z;

    _offSet(0) = volume.offset().x;
    _offSet(1) = volume.offset().y;
    _offSet(2) = volume.offset().z;
}

} // namespace mat
} // namespace CTL

#endif // PMATCOMPARATOR_H
