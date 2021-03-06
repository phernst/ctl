#include "pmatcomparator.h"
#include <algorithm>

namespace CTL {
namespace mat {

PMatComparator::PMatComparator(const QSize& nbDetectorPixels)
    : _nbPixels{ uint(nbDetectorPixels.width()), uint(nbDetectorPixels.height()) }
{
}

PMatComparator::PMatComparator(uint nbDetectorPixelsX, uint nbDetectorPixelsY)
    : _nbPixels{ nbDetectorPixelsX, nbDetectorPixelsY }
{
}

PMatComparator::PMatComparator(const QSize& nbDetectorPixels,
                               double totVolumeSizeX, double totVolumeSizeY, double totVolumeSizeZ,
                               double accuracy)
    : PMatComparator(nbDetectorPixels)
{
    this->setTotalVolumeSize(totVolumeSizeX, totVolumeSizeY, totVolumeSizeZ);
    if(accuracy != 1.0)
        this->setAccuracy(accuracy);
}

PMatComparator::PMatComparator(uint nbDetectorPixelsX, uint nbDetectorPixelsY,
                               double totVolumeSizeX, double totVolumeSizeY, double totVolumeSizeZ,
                               double accuracy)
    : PMatComparator(nbDetectorPixelsX, nbDetectorPixelsY)
{
    this->setTotalVolumeSize(totVolumeSizeX, totVolumeSizeY, totVolumeSizeZ);
    if(accuracy != 1.0)
        this->setAccuracy(accuracy);
}

PMatComparator::Eval PMatComparator::operator()(const ProjectionMatrix& P1,
                                                const ProjectionMatrix& P2) const
{
    Eval ret;
    uint X = _nbVoxels[0], Y = _nbVoxels[1], Z = _nbVoxels[2]; // abbreviation

    // center of the voxel that is at the corner of the volume
    Vector3x1 volCorner{ -0.5 * _voxelSize.get<0>() * double(X - 1),
                         -0.5 * _voxelSize.get<1>() * double(Y - 1),
                         -0.5 * _voxelSize.get<2>() * double(Z - 1) };
    volCorner += _offSet;
    // upper bound for projection pixel index (nbPixels - 1 + 0.5, this bound is because
    // integer index is considered to be the center of a pixel)
    const double upperBoundX = double(_nbPixels[0]) - 0.5;
    const double upperBoundY = double(_nbPixels[1]) - 0.5;

    // temp variables within the following loop
    double norm;
    Vector3x1 p1_homo, p1_homoX, p1_homoY, p2_homo, p2_homoX, p2_homoY;
    Matrix<2, 1> diff, p1, p2;
    // 3d world coord of voxel center (homog. form)
    Matrix<4, 1> r({ 0.0, 0.0, 0.0, 1.0 });
    // the projection matrices splitted into columns
    const Vector3x1 P1Column0 = P1.column<0>();
    const Vector3x1 P1Column1 = P1.column<1>();
    const Vector3x1 P1Column2 = P1.column<2>();
    const Vector3x1 P1Column3 = P1.column<3>();
    const Vector3x1 P2Column0 = P2.column<0>();
    const Vector3x1 P2Column1 = P2.column<1>();
    const Vector3x1 P2Column2 = P2.column<2>();
    const Vector3x1 P2Column3 = P2.column<3>();

    // __Iteration over voxels__
    // use separation of matrix product:
    // (P*r)(i) = P(i,0)*r(0) + P(i,1)*r(1) + P(i,2)*r(2) + P(i,3)*r(3)
    for(uint x = 0; x < X; ++x)
    {
        r.get<0>() = std::fma(double(x), _voxelSize.get<0>(), volCorner.get<0>());
        // P*r = P(.,0)*r(0) + P(.,3)*r(3) + ...
        p1_homoX = P1Column0 * r.get<0>() + P1Column3; // * r(3)=1.0
        p2_homoX = P2Column0 * r.get<0>() + P2Column3; // * r(3)=1.0

        for(uint y = 0; y < Y; ++y)
        {
            r.get<1>() = std::fma(double(y), _voxelSize.get<1>(), volCorner.get<1>());
            // ... + P(.,1)*r(1)
            p1_homoY = p1_homoX + P1Column1 * r.get<1>();
            p2_homoY = p2_homoX + P2Column1 * r.get<1>();

            for(uint z = 0; z < Z; ++z)
            {
                r.get<2>() = std::fma(double(z), _voxelSize.get<2>(), volCorner.get<2>());
                // ... + P(.,2)*r(2)
                p1_homo = p1_homoY + P1Column2 * r.get<2>();
                p2_homo = p2_homoY + P2Column2 * r.get<2>();

                // convert to cartesian coord (divide by w, where p_homo = [x y w])
                p1 = { p1_homo.get<0>(), p1_homo.get<1>() };
                p2 = { p2_homo.get<0>(), p2_homo.get<1>() };
                p1 /= p1_homo.get<2>();
                p2 /= p2_homo.get<2>();

                // check if p1 or p2 is outside the detector
                if(_restrictToDetectorArea)
                    if(p1.get<0>() < -0.5 || p1.get<0>() > upperBoundX ||
                       p1.get<1>() < -0.5 || p1.get<1>() > upperBoundY ||
                       p2.get<0>() < -0.5 || p2.get<0>() > upperBoundX ||
                       p2.get<1>() < -0.5 || p2.get<1>() > upperBoundY)
                        continue;

                // projection error
                diff = p1 - p2;
                norm = diff.norm();
                // statistical quantities
                ret.meanError += norm;
                ret.minError = std::min(ret.minError, norm);
                ret.maxError = std::max(ret.maxError, norm);
                if(_enableStandardDeviation)
                    ret.stdDeviation += norm * norm;
                ++ret.samples;
            }
        }
    }
    auto N = double(ret.samples);
    if(_enableStandardDeviation)
    {
        ret.stdDeviation -= ret.meanError * ret.meanError / N;
        ret.stdDeviation = std::sqrt(ret.stdDeviation / N);
    }
    ret.meanError /= N;

    return ret;
}

bool PMatComparator::computeStandardDeviation() const
{
    return _enableStandardDeviation;
}

bool PMatComparator::restrictedToDetectorArea() const
{
    return _restrictToDetectorArea;
}

void PMatComparator::setRestrictionToDetectorArea(bool restrictionEnabled)
{
    _restrictToDetectorArea = restrictionEnabled;
}

void PMatComparator::setComputeStandardDeviation(bool standardDeviationEnabled)
{
    _enableStandardDeviation = standardDeviationEnabled;
}

void PMatComparator::setTotalVolumeSize(double totVolumeSizeX,
                                        double totVolumeSizeY,
                                        double totVolumeSizeZ)
{
    _voxelSize = { totVolumeSizeX / _nbVoxels[0],
                   totVolumeSizeY / _nbVoxels[1],
                   totVolumeSizeZ / _nbVoxels[2] };
}

void PMatComparator::setAccuracy(double accuracy)
{
    auto totVolSize = totalVolumeSize();
    auto nbVoxels = qMax(uint(qRound(DEFAULT_NB_VOXELS * accuracy)), 1u);
    _nbVoxels[0] = nbVoxels;
    _nbVoxels[1] = nbVoxels;
    _nbVoxels[2] = nbVoxels;
    _voxelSize = { totVolSize.get<0>() / nbVoxels,
                   totVolSize.get<1>() / nbVoxels,
                   totVolSize.get<2>() / nbVoxels };
}

void PMatComparator::setVolumeGridSpacing(const Vector3x1& voxelSize)
{
    setVolumeGridSpacing(voxelSize.get<0>(), voxelSize.get<1>(), voxelSize.get<2>());
}

void PMatComparator::setVolumeGridSpacing(double voxelSizeX, double voxelSizeY, double voxelSizeZ)
{
    Q_ASSERT(voxelSizeX != 0.0);
    Q_ASSERT(voxelSizeY != 0.0);
    Q_ASSERT(voxelSizeZ != 0.0);
    auto totVolSize = totalVolumeSize();
    _nbVoxels[0] = uint(qRound(totVolSize.get<0>() / voxelSizeX));
    _nbVoxels[1] = uint(qRound(totVolSize.get<1>() / voxelSizeY));
    _nbVoxels[2] = uint(qRound(totVolSize.get<2>() / voxelSizeZ));
    _voxelSize = { voxelSizeX, voxelSizeY, voxelSizeZ };
}

void PMatComparator::setVolumeOffSet(const Vector3x1& offSet)
{
    setVolumeOffSet(offSet.get<0>(), offSet.get<1>(), offSet.get<2>());
}

void PMatComparator::setVolumeOffSet(double offSetX, double offSetY, double offSetZ)
{
    _offSet.get<0>() = offSetX;
    _offSet.get<1>() = offSetY;
    _offSet.get<2>() = offSetZ;
}

void PMatComparator::setTotalVolumeSize(const Vector3x1& totVolumeSize)
{
    _offSet = totVolumeSize;
}

const Vector3x1 &PMatComparator::volumeGridSpacing() const
{
    return _voxelSize;
}

Vector3x1 PMatComparator::totalVolumeSize() const
{
    return { _voxelSize.get<0>() * _nbVoxels[0],
             _voxelSize.get<1>() * _nbVoxels[1],
             _voxelSize.get<2>() * _nbVoxels[2] };
}

const Vector3x1& PMatComparator::volumeOffset() const
{
    return _offSet;
}

void PMatComparator::setNumberDetectorPixels(const QSize& nbPixels)
{
    _nbPixels[0] = nbPixels.width();
    _nbPixels[1] = nbPixels.height();
}

void PMatComparator::setNumberDetectorPixels(uint x, uint y)
{
    _nbPixels[0] = x;
    _nbPixels[1] = y;
}

} // namespace mat
} // namespace CTL
