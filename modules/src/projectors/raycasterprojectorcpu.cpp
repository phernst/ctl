#include "raycasterprojectorcpu.h"
#include "acquisition/geometryencoder.h"
#include "components/abstractdetector.h"
#include "mat/matrix_algorithm.h"
#include "processing/threadpool.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(RayCasterProjectorCPU);

namespace {
// helper functions
mat::Matrix<4,4> decomposeM(const Matrix3x3& M);
mat::Matrix<3,1> volumeCorner(const VoxelVolume<float>& volume);
// normalized direction vector (world coord. frame) to detector pixel [x,y]
mat::Matrix<3,1> calculateDirection(double x, double y, const mat::Matrix<4,4>& QR);
// parameters of the ray (specified by `source`and `direction`) for the entry and exit point
mat::Matrix<2,1> calculateIntersections(const mat::Matrix<3,1>& source,
                                        const mat::Matrix<3,1>& direction,
                                        const mat::Matrix<3,1>& volSize,
                                        const mat::Matrix<3,1>& volCorner,
                                        bool interpolate);
template<uint,uint>
mat::Matrix<2,1> calculateHit(const mat::Matrix<3,1>& source, const mat::Matrix<3,1>& lambda,
                              const mat::Matrix<3,1>& direction);
// helper for `calculateIntersections`: checks `lambda` as a candidate for a parameter of entry/exit
// point compared to given `minMax` values for a certain face of the volume. `corner1` and `corner2`
// are the corners of the 2d face and `hit` is the intersection of the ray with the face.
template<uint,uint>
mat::Matrix<2,1> checkFace(const mat::Matrix<2,1>& hit, const mat::Matrix<3,1>& corner1,
                           const mat::Matrix<3,1>& corner2, const mat::Matrix<3,1>& lambda,
                           const mat::Matrix<2,1>& minMax);
float interpolatedRead(const VolumeData& volume,
                       const mat::Matrix<3,1>& position);
float nonInterpolatedRead(const VolumeData& volume,
                          const mat::Matrix<3,1>& position);

} // unnamed namespace

/*!
 * Configures the projector. This extracts all information that is required for projecting with
 * this projector from the \a setup.
 */
void RayCasterProjectorCPU::configure(const AcquisitionSetup& setup)
{
    // get projection matrices
    _pMats = GeometryEncoder::encodeFullGeometry(setup);

    // extract required system geometry
    _viewDim = setup.system()->detector()->viewDimensions();
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
    // allocate projections
    ret.allocateMemory(nbViews);

    // Prepare input data
    auto volCorner = volumeCorner(volume);

    // define projection task for each view
    ThreadPool tp;
    auto threadTask = [&volume, &volCorner, this] (SingleViewData* proj, uint view) {
        *proj = computeView(volume, volCorner, view);
    };
    // loop over all views
    for(auto view = 0u; view < nbViews; ++view)
    {
        tp.enqueueThread(threadTask, &ret.view(view), view);
        emit notifier()->projectionFinished(int(view));
    }

    return ret;
}

RayCasterProjectorCPU::Settings& RayCasterProjectorCPU::settings()
{
    return _settings;
}

// Use SerializationInterface::toVariant() documentation.
QVariant RayCasterProjectorCPU::toVariant() const
{
    QVariantMap ret = AbstractProjector::toVariant().toMap();

    ret.insert("#", "RayCasterProjectorCPU");

    return ret;
}

QVariant RayCasterProjectorCPU::parameter() const
{
    QVariantMap ret = AbstractProjector::parameter().toMap();

    ret.insert("Rays per pixel X", _settings.raysPerPixel[0]);
    ret.insert("Rays per pixel Y", _settings.raysPerPixel[1]);
    ret.insert("Ray sampling step length", _settings.raySampling);
    ret.insert("Interpolate", _settings.interpolate);

    return ret;
}

void RayCasterProjectorCPU::setParameter(const QVariant& parameter)
{
    QVariantMap map = parameter.toMap();

    _settings.raysPerPixel[0] = map.value("Rays per pixel X", 1u).toUInt();
    _settings.raysPerPixel[1] = map.value("Rays per pixel Y", 1u).toUInt();
    _settings.raySampling = map.value("Ray sampling step length", 0.3f).toFloat();
    _settings.interpolate = map.value("Interpolate", true).toBool();
}

SingleViewData RayCasterProjectorCPU::computeView(const VolumeData& volume,
                                                  const mat::Matrix<3,1>& volumeCorner,
                                                  uint view) const
{  
    // sizes
    const uint detectorColumns = _viewDim.nbChannels;
    const uint detectorRows = _viewDim.nbRows;
    const uint detectorModules = _viewDim.nbModules;

    auto projection = SingleViewData(detectorColumns, detectorRows);
    projection.allocateMemory(detectorModules);

    // determine ray step length in mm
    const auto increment_mm = volume.smallestVoxelSize() * _settings.raySampling;

    // geometry
    const auto& currentViewPMats = _pMats.at(view);
    // all modules have same source position --> use first module PMat (arbitrary)
    const auto sourcePosition = currentViewPMats.first().sourcePosition();
    // individual module geometry: QR is only determined by M, where P=[M|p4]
    std::vector<mat::Matrix<4,4>> QRs(_viewDim.nbModules);
    for(auto module = 0u; module < _viewDim.nbModules; ++module)
        QRs[module] = decomposeM(currentViewPMats.at(module).M());

    // quantities normalized by the voxel size (units of "voxel numbers")
    const auto& voxelSize_mm = volume.voxelSize();
    const mat::Matrix<3,1> volSize(static_cast<double>(volume.dimensions().x),
                                   static_cast<double>(volume.dimensions().y),
                                   static_cast<double>(volume.dimensions().z));
    const mat::Matrix<3,1> source(sourcePosition(0) / voxelSize_mm.x,
                                  sourcePosition(1) / voxelSize_mm.y,
                                  sourcePosition(2) / voxelSize_mm.z);
    const mat::Matrix<3,1> volCorner(volumeCorner(0) / voxelSize_mm.x,
                                     volumeCorner(1) / voxelSize_mm.y,
                                     volumeCorner(2) / voxelSize_mm.z);
    const mat::Matrix<3,1> increment_vox(increment_mm / voxelSize_mm.x,
                                         increment_mm / voxelSize_mm.y,
                                         increment_mm / voxelSize_mm.z);

    //const float3 cornerToSourceVector = source - volCorner;
    const mat::Matrix<3,1> cornerToSourceVector = source - volCorner;

    // quantities related to the projection image pixels
    const std::array<uint,2> raysPerPixel { _settings.raysPerPixel[0], _settings.raysPerPixel[1] };
    const auto totalRaysPerPixel = static_cast<float>(raysPerPixel[0] * raysPerPixel[1]);
    const mat::Matrix<2,1> intraPixelSpacing(1.0 / raysPerPixel[0],
                                             1.0 / raysPerPixel[1]);

    // sampling method (interpolation on/off)
    float (*readValue)(const VolumeData&, const mat::Matrix<3,1>&);
    readValue = _settings.interpolate ? interpolatedRead
                                      : nonInterpolatedRead;

    // loop over all pixels of all modules
    for(auto module = 0u; module < detectorModules; ++module)
        for(auto x = 0u; x < detectorColumns; ++x)
            for(auto y = 0u; y < detectorRows; ++y)
            {
                const auto pixelCornerPlusOffset = mat::Matrix<2,1>(static_cast<double>(x) - 0.5,
                                                                    static_cast<double>(y) - 0.5)
                                                   + 0.5 * intraPixelSpacing;
                // resulting projection value
                double projVal = 0.0;

                // loop over sub-rays
                for(auto rayX = 0u; rayX < raysPerPixel[0]; ++rayX)
                    for(auto rayY = 0u; rayY < raysPerPixel[1]; ++rayY)
                    {
                        // helper variables
                        mat::Matrix<3,1> direction;
                        mat::Matrix<2,1> rayBounds, pixelCoord;

                        pixelCoord = pixelCornerPlusOffset
                            + mat::Matrix<2,1>(static_cast<double>(rayX) * intraPixelSpacing(0),
                                               static_cast<double>(rayY) * intraPixelSpacing(1));

                        direction = calculateDirection(pixelCoord(0), pixelCoord(1), QRs[module]);
                        direction(0) *= increment_vox(0);
                        direction(1) *= increment_vox(1);
                        direction(2) *= increment_vox(2);

                        rayBounds = calculateIntersections(source, direction, volSize, volCorner, _settings.interpolate);

                        // trace the ray
                        for(auto i   = static_cast<uint>(rayBounds(0)),
                                 end = static_cast<uint>(rayBounds(1)) + 1;
                            i <= end; ++i)
                        {
                            // position in volume
                            mat::Matrix<3,1> position;

                            position(0) = std::fma(static_cast<double>(i), direction(0), cornerToSourceVector(0));
                            position(1) = std::fma(static_cast<double>(i), direction(1), cornerToSourceVector(1));
                            position(2) = std::fma(static_cast<double>(i), direction(2), cornerToSourceVector(2));

                            projVal += static_cast<double>(readValue(volume, position));
                        }
                    }

                projection.module(module)(x,y) = increment_mm * static_cast<float>(projVal) / totalRaysPerPixel;
            }

    return projection;
}

namespace {

float interpolatedRead(const VolumeData& volume, const mat::Matrix<3, 1>& position)
{
    const auto& nbVoxels = volume.dimensions();
    if(position(0) < -0.5           || position(1) < -0.5           || position(2) < -0.5 ||
       position(0) > nbVoxels.x+0.5 || position(1) > nbVoxels.y+0.5 || position(2) > nbVoxels.z+0.5)
        return 0.0;

    // voxel 000 (smallest indices) -> subtract 0.5 to get "left-most, bottom voxel on the front"
    const std::array<int,3> vox { static_cast<int>(std::floor(position(0) - 0.5)),
                                  static_cast<int>(std::floor(position(1) - 0.5)),
                                  static_cast<int>(std::floor(position(2) - 0.5)) };

    // check if a border voxel is involved
    const std::array<bool, 3> borderIdxLow( { vox[0] < 0,
                                              vox[1] < 0,
                                              vox[2] < 0 } );
    const std::array<bool, 3> borderIdxHigh( { vox[0] >= int(nbVoxels.x) - 1,
                                               vox[1] >= int(nbVoxels.y) - 1,
                                               vox[2] >= int(nbVoxels.z) - 1  } );

    // compute weight factors
    const std::array<float, 3> weights { float(position(0) - (vox[0] + 0.5)),
                                         float(position(1) - (vox[1] + 0.5)),
                                         float(position(2) - (vox[2] + 0.5)) };

    // read out values of all 8 voxels (or assign zero if border)
    const auto v000 = (borderIdxLow[0]  || borderIdxLow[1]  || borderIdxLow[2])  ? 0.0f : volume(vox[0],   vox[1],   vox[2]);
    const auto v001 = (borderIdxLow[0]  || borderIdxLow[1]  || borderIdxHigh[2]) ? 0.0f : volume(vox[0],   vox[1],   vox[2]+1);
    const auto v010 = (borderIdxLow[0]  || borderIdxHigh[1] || borderIdxLow[2])  ? 0.0f : volume(vox[0],   vox[1]+1, vox[2]);
    const auto v011 = (borderIdxLow[0]  || borderIdxHigh[1] || borderIdxHigh[2]) ? 0.0f : volume(vox[0],   vox[1]+1, vox[2]+1);
    const auto v100 = (borderIdxHigh[0] || borderIdxLow[1]  || borderIdxLow[2])  ? 0.0f : volume(vox[0]+1, vox[1],   vox[2]);
    const auto v101 = (borderIdxHigh[0] || borderIdxLow[1]  || borderIdxHigh[2]) ? 0.0f : volume(vox[0]+1, vox[1],   vox[2]+1);
    const auto v110 = (borderIdxHigh[0] || borderIdxHigh[1] || borderIdxLow[2])  ? 0.0f : volume(vox[0]+1, vox[1]+1, vox[2]);
    const auto v111 = (borderIdxHigh[0] || borderIdxHigh[1] || borderIdxHigh[2]) ? 0.0f : volume(vox[0]+1, vox[1]+1, vox[2]+1);

    const auto w0_opp = 1.0f - weights[0];
    const auto c00 = w0_opp * v000 + weights[0] * v100;
    const auto c01 = w0_opp * v001 + weights[0] * v101;
    const auto c10 = w0_opp * v010 + weights[0] * v110;
    const auto c11 = w0_opp * v011 + weights[0] * v111;

    const auto w1_opp = 1.0f - weights[1];
    const auto c0 = c00 * w1_opp + c10 * weights[1];
    const auto c1 = c01 * w1_opp + c11 * weights[1];

    return c0 * (1.0f - weights[2]) + c1 * weights[2];
}

float nonInterpolatedRead(const VolumeData& volume, const mat::Matrix<3, 1>& position)
{
    const auto& volDim = volume.dimensions();
    if(position(0) < 0.0      || position(1) < 0.0      || position(2) < 0.0 ||
       position(0) > volDim.x || position(1) > volDim.y || position(2) > volDim.z)
        return 0.0;

    const std::array<int,3> voxelIdx { static_cast<int>(std::floor(position(0))),
                                       static_cast<int>(std::floor(position(1))),
                                       static_cast<int>(std::floor(position(2))) };

    return volume(voxelIdx[0], voxelIdx[1], voxelIdx[2]);
}


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

mat::Matrix<3,1> volumeCorner(const VoxelVolume<float>& volume)
{
    const auto& volDim = volume.dimensions();
    const auto& volOffset = volume.offset();
    const auto& voxelSize = volume.voxelSize();

    return { volOffset.x - 0.5f * static_cast<float>(volDim.x) * voxelSize.x,
             volOffset.y - 0.5f * static_cast<float>(volDim.y) * voxelSize.y,
             volOffset.z - 0.5f * static_cast<float>(volDim.z) * voxelSize.z };
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
                                        const mat::Matrix<3,1>& volCorner,
                                        bool interpolate)
{
    mat::Matrix<3,1> corner1 = volCorner;
    mat::Matrix<3,1> corner2 = volCorner + volSize;

    if(interpolate)
    {
        corner1 -= mat::Matrix<3,1>(0.5);
        corner2 += mat::Matrix<3,1>(0.5);
    }

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

    // # lambda1: faces around corner1
    // yz-face
    hit = calculateHit<1,2>(source, lambda1, direction);
    minMax = checkFace<1,2>(hit, corner1, corner2, lambda1, minMax);

    // xz-face
    hit = calculateHit<0,2>(source, lambda1, direction);
    minMax = checkFace<0,2>(hit, corner1, corner2, lambda1, minMax);

    // xy-face
    hit = calculateHit<0,1>(source, lambda1, direction);
    minMax = checkFace<0,1>(hit, corner1, corner2, lambda1, minMax);

    // # lambda2: faces around corner2
    // yz-face
    hit = calculateHit<1,2>(source, lambda2, direction);
    minMax = checkFace<1,2>(hit, corner1, corner2, lambda2, minMax);

    // xz-face
    hit = calculateHit<0,2>(source, lambda2, direction);
    minMax = checkFace<0,2>(hit, corner1, corner2, lambda2, minMax);

    // xy-face
    hit = calculateHit<0,1>(source, lambda2, direction);
    minMax = checkFace<0,1>(hit, corner1, corner2, lambda2, minMax);

    // enforce positivity (ray needs to start from the source)
    return { std::max( { minMax.get<0>(), 0.0 } ),
             std::max( { minMax.get<1>(), 0.0 } ) };
}

template<uint faceDim1, uint faceDim2>
mat::Matrix<2,1> calculateHit(const mat::Matrix<3,1>& source, const mat::Matrix<3,1>& lambda,
                              const mat::Matrix<3,1>& direction)
{
    constexpr auto orthoDim = 3u - faceDim1 - faceDim2;

    return mat::Matrix<2,1>(source.get<faceDim1>() + lambda.get<orthoDim>() * direction.get<faceDim1>(),
                            source.get<faceDim2>() + lambda.get<orthoDim>() * direction.get<faceDim2>());
}

template<uint faceDim1, uint faceDim2>
mat::Matrix<2,1> checkFace(const mat::Matrix<2,1>& hit, const mat::Matrix<3,1>& corner1,
                           const mat::Matrix<3,1>& corner2, const mat::Matrix<3,1>& lambda,
                           const mat::Matrix<2,1>& minMax)
{
    static auto greaterThanZero = [] (const int& val) { return val > 0; };

    constexpr auto orthoDim = 3u - faceDim1 - faceDim2;
    const auto lambdaVal = lambda.get<orthoDim>();

    // basic condition: ray must hit the volume (must not pass by)
    const std::vector<int> compares{ hit.get<0>() >= corner1.get<faceDim1>(),
                                     hit.get<1>() >= corner1.get<faceDim2>(),
                                     hit.get<0>() <= corner2.get<faceDim1>(),
                                     hit.get<1>() <= corner2.get<faceDim2>() };
    const int intersects = std::all_of(compares.cbegin(), compares.cend(), greaterThanZero);

    // check for intersection and if new `lambda` is less/greater then `minMax`
    const bool conditions1 = intersects && lambdaVal < minMax.get<0>();
    const bool conditions2 = intersects && lambdaVal > minMax.get<1>();

    // select new `lambda` if a condition is true, otherwise return old `minMax`
    return { conditions1 ? lambdaVal : minMax.get<0>(),
             conditions2 ? lambdaVal : minMax.get<1>() };
}

} // unnamed namespace

} // namespace CTL
