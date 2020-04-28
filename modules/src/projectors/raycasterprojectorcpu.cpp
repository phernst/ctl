#include "raycasterprojectorcpu.h"
#include "acquisition/geometryencoder.h"
#include "components/abstractdetector.h"
#include "mat/matrix_algorithm.h"
#include "processing/threadpool.h"

#include <array>

namespace CTL {

namespace {
// helper functions
mat::Matrix<4,4> decomposeM(const Matrix3x3& M);
mat::Matrix<3,1> determineSource(const ProjectionMatrix& P);
mat::Matrix<3,1> volumeCorner(const VoxelVolume<float>::Dimensions& volDim,
                              const VoxelVolume<float>::VoxelSize& voxelSize,
                              const VoxelVolume<float>::Offset& volOffset);
// normalized direction vector (world coord. frame) to detector pixel [x,y]
mat::Matrix<3,1> calculateDirection(double x, double y, const mat::Matrix<4,4>& QR);
// parameters of the ray (specified by `source`and `direction`) for the entry and exit point
mat::Matrix<2,1> calculateIntersections(const mat::Matrix<3,1>& source,
                                        const mat::Matrix<3,1>& direction,
                                        const mat::Matrix<3,1>& volSize,
                                        const mat::Matrix<3,1>& volCorner);
// helper for `calculateIntersections`: checks `lambda` as a candidate for a parameter of entry/exit
// point compared to given `minMax` values for a certain face of the volume. `corner1` and `corner2`
// are the corners of the 2d face and `hit` is the intersection of the ray with the face.
mat::Matrix<2,1> checkFace(const mat::Matrix<2,1>& hit, const mat::Matrix<2,1>& corner1,
                           const mat::Matrix<2,1>& corner2, float lambda,
                           const mat::Matrix<2,1>& minMax);

}

/*!
 * Configures the projector. This extracts all information that is required for projecting with
 * this projector from the \a setup.
 */
void RayCasterProjectorCPU::configure(const AcquisitionSetup& setup)
{
    // get projection matrices
    _pMats = GeometryEncoder::encodeFullGeometry(setup);

    // extract required system geometry
    auto detectorPixels = setup.system()->detector()->nbPixelPerModule();
    _viewDim.nbRows = uint(detectorPixels.height());
    _viewDim.nbChannels = uint(detectorPixels.width());
    _viewDim.nbModules = setup.system()->detector()->nbDetectorModules();
}

/*!
 * Computes the projection of \a volume for all views that have been configured in the configure()
 * step. Returns projection data of all views and detector modules as a ProjectionData object.
 */
ProjectionData RayCasterProjectorCPU::project(const VolumeData& volume)
{
    // the returned object
    ProjectionData ret(_viewDim);
    // check for a valid volume
    if(!volume.hasData())
    {
        qCritical() << "no or contradictory data in volume object";
        return ret;
    }
    if(volume.smallestVoxelSize() <= 0.0f)
        qWarning() << "voxel size is zero or negative";

    // projection dimensions
    const auto nbViews = _pMats.size();

    // volume specs
    const auto& volDim = volume.dimensions();
    const auto& volOffset = volume.offset();
    const auto& voxelSize = volume.voxelSize();

    // determine ray step length in mm
    auto smallestVoxelSize = std::min( { voxelSize.x, voxelSize.y, voxelSize.z } );
    auto increment_mm = smallestVoxelSize * _settings.raySampling;

    // Prepare input data
    QPair<uint, uint> raysPerPixel { _settings.raysPerPixel[0], _settings.raysPerPixel[1] };
    auto volCorner = volumeCorner(volDim, voxelSize, volOffset);

    // loop over all projections
    ThreadPool tp;
    ret.allocateMemory(nbViews);
    auto threadTask = [&volume, &volDim, &voxelSize, &volCorner, &raysPerPixel, increment_mm,
                       this](SingleViewData* proj, uint view) {
        *proj = computeView(volume, view, volDim, voxelSize, volCorner, raysPerPixel, increment_mm);
    };
    for(auto view = 0u; view < nbViews; ++view)
    {
        tp.enqueueThread(threadTask, &ret.view(view), view);

        emit notifier()->projectionFinished(int(view));
    }

    return ret;
}

SingleViewData RayCasterProjectorCPU::computeView(const VolumeData& volume,
                                                  uint view,
                                                  const VoxelVolume<float>::Dimensions& volDim,
                                                  const VoxelVolume<float>::VoxelSize& voxelSize_mm,
                                                  const mat::Matrix<3,1>& volumeCorner,
                                                  QPair<uint,uint> raysPerPixel,
                                                  float increment_mm) const
{
    const auto& currentViewPMats = _pMats.at(view);
    // all modules have same source position --> use first module PMat (arbitrary)
    auto sourcePosition = determineSource(currentViewPMats.first());
    // individual module geometry: QR is only determined by M, where P=[M|p4]
    std::vector<mat::Matrix<4,4>> QRs(_viewDim.nbModules);
    for(auto module = 0u; module < _viewDim.nbModules; ++module)
        QRs[module] = decomposeM(currentViewPMats.at(module).M());

    // sizes
    const uint detectorColumns = _viewDim.nbChannels;
    const uint detectorRows = _viewDim.nbRows;
    const uint detectorModules = _viewDim.nbModules;

    auto projection = SingleViewData(detectorColumns, detectorRows);
    projection.allocateMemory(detectorModules);

    // quantities normalized by the voxel size (units of "voxel numbers")
    const mat::Matrix<3,1> volSize(double(volDim.x), double(volDim.y), double(volDim.z));
    const mat::Matrix<3,1> source(sourcePosition(0) / voxelSize_mm.x,
                                  sourcePosition(1) / voxelSize_mm.y,
                                  sourcePosition(2) / voxelSize_mm.z);
    const mat::Matrix<3,1> volCorner(volumeCorner(0) / voxelSize_mm.x,
                                     volumeCorner(1) / voxelSize_mm.y,
                                     volumeCorner(2) / voxelSize_mm.z);

    //const float3 cornerToSourceVector = source - volCorner;
    const mat::Matrix<3,1> cornerToSourceVector = source - volCorner;

    // quantities related to the projection image pixels
    const mat::Matrix<2,1> intraPixelSpacing(1.0 / raysPerPixel.first,
                                             1.0 / raysPerPixel.second);

    const auto totalRaysPerPixel = float(raysPerPixel.first * raysPerPixel.second);

    auto greaterEqualZero = [] (const int& val) { return val >= 0; };

    for(uint module = 0; module < detectorModules; ++module)
        for(uint x = 0; x < detectorColumns; ++x)
        {
            for(uint y = 0; y < detectorRows; ++y)
            {
                const mat::Matrix<2,1> pixelCornerPlusOffset = mat::Matrix<2,1>((double)x - 0.5, (double)y - 0.5) + 0.5 * intraPixelSpacing;

                // helper variables
                mat::Matrix<3,1> direction, position;
                mat::Matrix<2,1> rayBounds, pixelCoord;
                std::array<int,3> voxelIdx;

                // resulting projection value
                double projVal = 0.0;
                for(uint rayX = 0; rayX < raysPerPixel.first; ++rayX)
                    for(uint rayY = 0; rayY < raysPerPixel.second; ++rayY)
                    {
                        pixelCoord = pixelCornerPlusOffset + mat::Matrix<2,1>((double)rayX * intraPixelSpacing(0),
                                                                              (double)rayY * intraPixelSpacing(1));

                        direction = increment_mm * calculateDirection(pixelCoord(0),
                                                                      pixelCoord(1),
                                                                      QRs[module]);
                        direction(0) /= voxelSize_mm.x;
                        direction(1) /= voxelSize_mm.y;
                        direction(2) /= voxelSize_mm.z;

                        rayBounds = calculateIntersections(source, direction, volSize, volCorner);

                        for(uint i = (uint)rayBounds(0), end = (uint)rayBounds(1)+1; i <= end; ++i)
                        {
                            position(0) = std::fma(double(i), direction(0), cornerToSourceVector(0));
                            position(1) = std::fma(double(i), direction(1), cornerToSourceVector(1));
                            position(2) = std::fma(double(i), direction(2), cornerToSourceVector(2));

                            voxelIdx[0] = static_cast<int>(std::floor(position(0)));
                            voxelIdx[1] = static_cast<int>(std::floor(position(1)));
                            voxelIdx[2] = static_cast<int>(std::floor(position(2)));

                            if(std::all_of(voxelIdx.cbegin(), voxelIdx.cend(), greaterEqualZero) &&
                                    uint(voxelIdx[0]) < volDim.x &&
                                    uint(voxelIdx[1]) < volDim.y &&
                                    uint(voxelIdx[2]) < volDim.z)
                            {
                                projVal += (double)volume(voxelIdx[0], voxelIdx[1], voxelIdx[2]);
                            }

                        }
                    }

                projection.module(module)(x,y) = increment_mm * (float)projVal / totalRaysPerPixel;

            }
        }

    return projection;
}

namespace {

// slightly earlier/later entry/exit point (1% of voxel size) allowed for numerical reasons
static const mat::Matrix<3,1> EPS(0.01, 0.01, 0.01);

mat::Matrix<4,4> decomposeM(const Matrix3x3& M)
{
    auto QR = mat::QRdecomposition(M);
    auto& Q = QR.Q;
    auto& R = QR.R;
    if(std::signbit(R(0, 0) * R(1, 1) * R(2, 2)))
        R = -R;
    mat::Matrix<4,4> ret =  { Q(0,0), Q(0,1), Q(0,2),
                              Q(1,0), Q(1,1), Q(1,2),
                              Q(2,0), Q(2,1), Q(2,2),
                              R(0,0), R(0,1), R(0,2),
                                      R(1,1), R(1,2),
                                              R(2,2),
                                              0.0} ;
    return ret;
}

mat::Matrix<3,1> determineSource(const ProjectionMatrix& P)
{
    return P.sourcePosition();
}

mat::Matrix<3,1> volumeCorner(const VoxelVolume<float>::Dimensions& volDim,
                              const VoxelVolume<float>::VoxelSize& voxelSize,
                              const VoxelVolume<float>::Offset& volOffset)
{
    return { volOffset.x - 0.5f * float(volDim.x) * voxelSize.x,
             volOffset.y - 0.5f * float(volDim.y) * voxelSize.y,
             volOffset.z - 0.5f * float(volDim.z) * voxelSize.z };
}

mat::Matrix<3,1> calculateDirection(double x, double y, const mat::Matrix<4,4>& QR)
{
    // Q^t*[x,y,1]
    const auto Qtx = mat::Matrix<3,1>( QR(0)*x + QR(3)*y + QR(6),
                                       QR(1)*x + QR(4)*y + QR(7),
                                       QR(2)*x + QR(5)*y + QR(8) );
    // R^-1*Qtx
    const double dz = Qtx(2) / QR(14);
    const double dy = (Qtx(1) - dz*QR(13)) / QR(12);
    const double dx = (Qtx(0) - dy*QR(10) - dz*QR(11)) / QR(9);

    return mat::Matrix<3,1>(dx, dy, dz).normalized();
}

mat::Matrix<2,1> calculateIntersections(const mat::Matrix<3,1>& source,
                                        const mat::Matrix<3,1>& direction,
                                        const mat::Matrix<3,1>& volSize,
                                        const mat::Matrix<3,1>& volCorner)
{
    mat::Matrix<3,1> corner1 = volCorner;
    mat::Matrix<3,1> corner2 = volCorner + volSize;

    // intersection of the ray with all six planes/faces of the volume
    const mat::Matrix<3,1> lambda1 = { (corner1 - source)(0) / direction(0),
                                       (corner1 - source)(1) / direction(1),
                                       (corner1 - source)(2) / direction(2) };
    const mat::Matrix<3,1> lambda2 = { (corner2 - source)(0) / direction(0),
                                       (corner2 - source)(1) / direction(1),
                                       (corner2 - source)(2) / direction(2) };

    // relax boundary conditions
    corner1 -= EPS;
    corner2 += EPS;

    // find the two intersections within the volume boundaries (entry/exit)
    mat::Matrix<2,1> minMax(std::numeric_limits<double>::max(), 0.0);
    mat::Matrix<2,1> hit;

    auto matrixSel = [] (const mat::Matrix<3,1>& mat, uint dim1, uint dim2)
    { return mat::Matrix<2,1>{ mat(dim1), mat(dim2) }; };

    // # lambda1: faces around corner1
    // yz-face
    hit = matrixSel(source, 1, 2) + lambda1(0) * matrixSel(direction, 1, 2);
    minMax = checkFace(hit, matrixSel(corner1, 1, 2), matrixSel(corner2, 1, 2), lambda1(0), minMax);

    // xz-face
    hit = matrixSel(source, 0, 2) + lambda1(1) * matrixSel(direction, 0, 2);
    minMax = checkFace(hit, matrixSel(corner1, 0, 2), matrixSel(corner2, 0, 2), lambda1(1), minMax);

    // xy-face
    hit = matrixSel(source, 0, 1) + lambda1(2) * matrixSel(direction, 0, 1);
    minMax = checkFace(hit, matrixSel(corner1, 0, 1), matrixSel(corner2, 0, 1), lambda1(2), minMax);

    // # lambda2: faces around corner2
    // yz-face
    hit = matrixSel(source, 1, 2) + lambda2(0) * matrixSel(direction, 1, 2);
    minMax = checkFace(hit, matrixSel(corner1, 1, 2), matrixSel(corner2, 1, 2), lambda2(0), minMax);

    // xz-face
    hit = matrixSel(source, 0, 2) + lambda2(1) * matrixSel(direction, 0, 2);
    minMax = checkFace(hit, matrixSel(corner1, 0, 2), matrixSel(corner2, 0, 2), lambda2(1), minMax);

    // xy-face
    hit = matrixSel(source, 0, 1) + lambda2(2) * matrixSel(direction, 0, 1);
    minMax = checkFace(hit, matrixSel(corner1, 0, 1), matrixSel(corner2, 0, 1), lambda2(2), minMax);

    // enforce positivity (ray needs to start from the source)
    return { std::max( { minMax(0), 0.0 } ),
             std::max( { minMax(1), 0.0 } ) };
}

mat::Matrix<2,1> checkFace(const mat::Matrix<2,1>& hit, const mat::Matrix<2,1>& corner1,
                           const mat::Matrix<2,1>& corner2, float lambda,
                           const mat::Matrix<2,1>& minMax)
{
    // basic condition: ray must hit the volume (must not pass by)
    const std::vector<int> compares{ hit(0) >= corner1(0),
                                     hit(1) >= corner1(1),
                                     hit(0) <= corner2(0),
                                     hit(1) <= corner2(1) };
    const int intersects = std::all_of(compares.cbegin(), compares.cend(), [] (const int& val) { return val > 0; } );

    // check for intersection and if new `lambda` is less/greater then `minMax`
    const bool conditions1 = intersects && lambda < minMax(0);
    const bool conditions2 = intersects && lambda > minMax(1);

    // select new `lambda` if a condition is true, otherwise return old `minMax`
    return { conditions1 ? lambda : minMax(0),
             conditions2 ? lambda : minMax(1) };
}

} // namespace (anonymous)

} // namespace CTL
