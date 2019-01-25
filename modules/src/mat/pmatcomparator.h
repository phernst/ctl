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

#include "projectionmatrix.h"
#include "img/voxelvolume.h"
#include <float.h>
#include <QSize>

typedef unsigned int uint;

namespace CTL {
namespace mat {

class PMatComparator
{
public:
    struct Eval {
        double meanError    = 0.0;
        double minError     = DBL_MAX;
        double maxError     = 0.0;
        double stdDeviation = 0.0;
        int64_t samples     = 0;
    };

    typedef Matrix<3,1> Vector;
    typedef uint Size3D[3];
    typedef uint Size2D[2];

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
    void setVolumeGridSpacing(const Vector& voxelSize);
    void setVolumeGridSpacing(double voxelSizeX, double voxelSizeY, double voxelSizeZ);
    void setVolumeOffSet(const Vector& offSet);
    void setVolumeOffSet(double offSetX, double offSetY, double offSetZ);
    void setTotalVolumeSize(const Vector& totVolumeSize);
    void setTotalVolumeSize(double totVolumeSizeX, double totVolumeSizeY, double totVolumeSizeZ);
    Vector totalVolumeSize() const;
    const Vector& volumeGridSpacing() const;
    const Vector& volumeOffset() const;

    // detector settings
    const QSize& numberDetectorPixels() const;
    void setNumberDetectorPixels(const QSize& nbPixels);
    void setNumberDetectorPixels(uint x, uint y);

private:
    // volume
    Size3D _nbVoxels{ DEFAULT_NB_VOXELS, DEFAULT_NB_VOXELS, DEFAULT_NB_VOXELS };
    Vector _voxelSize{ { 8.0, 8.0, 8.0 } };
    Vector _offSet{ { 0.0, 0.0, 0.0 } };

    // detector
    Size2D _nbPixels{ 640, 480 };
    bool _restrictToDetectorArea{ true };
    bool _enableStandardDeviation{ false };
};

} // namespace mat
} // namespace CTL

#endif // PMATCOMPARATOR_H
