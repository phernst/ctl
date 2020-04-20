#include "consistency.h"
#include "ocl/clfileloader.h"
#include "processing/radontransform2d.h"
#include "processing/radontransform3d.h"
#include "processing/imageprocessing.h"
#include "mat/projectionmatrix.h"
#include "mat/matrix.h"
#include "mat/mat.h"

#include <bitset>
#include <random>

const std::string CL_KERNEL_HOM2RADON = "homToRadon"; //!< name of the OpenCL kernel function
const std::string CL_KERNEL_RADON2HOM = "radonToHom"; //!< name of the OpenCL kernel function

namespace {

template <class T>
std::vector<T> randomSubset(std::vector<T>&& fullSamples, uint seed, float subsampleLevel);

} // unnamed namespace

namespace CTL {

// #### IntermediateFctPair ####
// -----------------------------

IntermediateFctPair::IntermediateFctPair(std::vector<float> first, std::vector<float> second,
                                         Type secondType)
    : _first(first.size() == second.size()
             ? std::make_shared<const std::vector<float>>(std::move(first))
             : std::make_shared<const std::vector<float>>())
    , _second(_first->size() == second.size()
              ? std::make_shared<const std::vector<float>>(std::move(second))
              : std::make_shared<const std::vector<float>>())
    , _secondType(secondType)
{
}

IntermediateFctPair::IntermediateFctPair(std::shared_ptr<const std::vector<float>> first,
                                         std::vector<float> second, Type secondType)
    : _first(first->size() == second.size()
             ? std::move(first)
             : std::make_shared<const std::vector<float>>())
    , _second(_first->size() == second.size()
              ? std::make_shared<const std::vector<float>>(std::move(second))
              : std::make_shared<const std::vector<float>>())
    , _secondType(secondType)
{
}

IntermediateFctPair::IntermediateFctPair(std::vector<float> first,
                                         std::shared_ptr<const std::vector<float>> second,
                                         Type secondType)
    : _first(first.size() == second->size()
             ? std::make_shared<const std::vector<float>>(std::move(first))
             : std::make_shared<const std::vector<float>>())
    , _second(_first->size() == second->size()
              ? std::move(second)
              : std::make_shared<const std::vector<float>>())
    , _secondType(secondType)
{
}

IntermediateFctPair::IntermediateFctPair(std::shared_ptr<const std::vector<float>> first,
                                         std::shared_ptr<const std::vector<float>> second,
                                         Type secondType)
    : _first(first->size() == second->size()
             ? std::move(first)
             : std::make_shared<const std::vector<float>>())
    , _second(_first->size() == second->size()
              ? std::move(second)
              : std::make_shared<const std::vector<float>>())
    , _secondType(secondType)
{
}

double IntermediateFctPair::inconsistency(const imgproc::AbstractErrorMetric& metric, bool swapInput) const
{
    Q_ASSERT(!isEmpty());
    return swapInput ? metric(*_second, *_first) : metric(*_first, *_second);
}

bool IntermediateFctPair::isEmpty() const
{
    return _first->empty();
}

const std::vector<float>& IntermediateFctPair::first() const { return *_first; }

const std::vector<float>& IntermediateFctPair::second() const { return *_second; }

const std::shared_ptr<const std::vector<float>>& IntermediateFctPair::ptrToFirst() const
{
    return _first;
}

const std::shared_ptr<const std::vector<float>>& IntermediateFctPair::ptrToSecond() const
{
    return _second;
}

namespace OCL {

// #### IntermedGen2D2D ####
// -------------------------

double IntermedGen2D2D::angleIncrement() const
{
    return _angleIncrement;
}

float IntermedGen2D2D::subsampleLevel() const
{
    return _subsampleLevel;
}

void IntermedGen2D2D::setAngleIncrement(double angleIncrement)
{
    _angleIncrement = angleIncrement;
}

void IntermedGen2D2D::setSubsampleLevel(float subsampleLevel)
{
    if(subsampleLevel <= 0.0f)
        qWarning("New subsampling level ignored, since it is negative or zero.");
    else if(subsampleLevel > 1.0f)
        qWarning("New subsampling level ignored, since it is greater than one.");
    else
    {
        _subsampleLevel = subsampleLevel;
        _useSubsampling = true;
    }
}

void IntermedGen2D2D::toggleSubsampling(bool enabled)
{
    _useSubsampling = enabled;
}

IntermediateFctPair IntermedGen2D2D::intermedFctPair(const Chunk2D<float>& proj1,
                                                     const mat::ProjectionMatrix& P1,
                                                     const Chunk2D<float>& proj2,
                                                     const mat::ProjectionMatrix& P2,
                                                     float plusMinusH) const
{
    if(proj1.dimensions() != proj2.dimensions())
        throw std::runtime_error("IntermedGen2D2D::intermedFctPair: size of projections must match.");

    auto radon2DCoords = linePairs(P1, P2, proj1.dimensions());

    if(_useSubsampling)
    {
        uint seed = std::random_device{}(); // pull random seed for subsampling
        radon2DCoords.first = randomSubset(std::move(radon2DCoords.first), seed, _subsampleLevel);
        radon2DCoords.second = randomSubset(std::move(radon2DCoords.second), seed, _subsampleLevel);
    }

    const IntermediateProj intermedFct1(proj1, P1.intrinsicMatK());
    const IntermediateProj intermedFct2(proj2, P2.intrinsicMatK());

    return { intermedFct1.sampled(radon2DCoords.first, plusMinusH),
             intermedFct2.sampled(radon2DCoords.second, plusMinusH),
             IntermediateFctPair::ProjectionDomain };
}

IntermediateFctPair IntermedGen2D2D::intermedFctPair(const ImageResampler& radon2dSampler1,
                                                     const mat::ProjectionMatrix& P1,
                                                     const ImageResampler& radon2dSampler2,
                                                     const mat::ProjectionMatrix& P2,
                                                     const Chunk2D<float>::Dimensions& projSize) const
{
    if(radon2dSampler1.imgDim() != radon2dSampler2.imgDim())
        throw std::runtime_error("IntermedGen2D2D::intermedFctPair: size of projections must match.");

    auto radon2DCoords = linePairs(P1, P2, projSize);

    if(_useSubsampling)
    {
        uint seed = std::random_device{}(); // pull random seed for subsampling
        radon2DCoords.first = randomSubset(std::move(radon2DCoords.first), seed, _subsampleLevel);
        radon2DCoords.second = randomSubset(std::move(radon2DCoords.second), seed, _subsampleLevel);
    }

    return { radon2dSampler1.sample(toGeneric2DCoord(radon2DCoords.first)),
             radon2dSampler2.sample(toGeneric2DCoord(radon2DCoords.second)),
             IntermediateFctPair::ProjectionDomain };
}

std::pair<IntermedGen2D2D::LineSet, IntermedGen2D2D::LineSet>
IntermedGen2D2D::linePairs(const mat::ProjectionMatrix& P1, const mat::ProjectionMatrix& P2,
                           const Chunk2D<float>::Dimensions& projSize,
                           const mat::Matrix<2, 1>& originRadon, double angleIncrement)
{
    static constexpr auto nbCorners = 4u;
    const std::array<mat::Matrix<2, 1>, nbCorners> detectorCorners = {
        mat::Matrix<2, 1>{ 0.0, 0.0 },
        mat::Matrix<2, 1>{ projSize.width - 1.0, 0.0 },
        mat::Matrix<2, 1>{ 0.0, projSize.height - 1.0 },
        mat::Matrix<2, 1>{ projSize.width - 1.0, projSize.height - 1.0 }
    };
    auto intersectsDetector = [&](const Radon2DCoord& line)
    {
        const mat::Matrix<1, 2> n2D = { std::cos(line.angle()), std::sin(line.angle()) };
        std::bitset<nbCorners> side;
        for(auto i = 0u; i < nbCorners; ++i)
            side.set(i, std::signbit(n2D * (detectorCorners[i] - originRadon) - line.dist()));
        return !(side.all() || side.none());
    };
    const auto source1 = P1.sourcePosition();
    const auto source2 = P2.sourcePosition();
    const auto M1 = P1.M(), M1transp = P1.M().transposed();
    const auto M2 = P2.M(), M2transp = P2.M().transposed();

    // vector parallel to the connection between the sources (baseline)
    auto baseLine = source2 - source1;
    const auto src2srcDistance = baseLine.norm();
    if(qFuzzyIsNull(src2srcDistance))
        throw std::runtime_error("IntermedGen2D2D::intermedFctPair: distance between the two source"
                                 " positions is close to zero.");
    baseLine /= src2srcDistance;

    // initialize plane normal
    const auto initNormal = orthonormalTo(baseLine);

    // rotate plane around baseline
    const auto nbRotAngles = size_t(PI / std::abs(angleIncrement) + 0.5);
    const auto negSource1 = -source1.transposed();
    const auto originRadonTransp = originRadon.transposed();
    LineSet lineSet1, lineSet2;

    for(size_t i = 0; i < nbRotAngles; ++i)
    {
        // plane `p` in homogeneous coordinates [nx, ny, nz, -d]
        const auto rotAngle = i * angleIncrement;
        const auto n = mat::rotationMatrix(rotAngle, baseLine) * initNormal;
        const auto p = mat::vertcat(n, negSource1 * n);

        // Pluecker matrix of the intersection line of `p` with the plane at infinity
        const mat::Matrix<3, 3> L{  0.0,  p(2), -p(1),
                                  -p(2),   0.0,  p(0),
                                   p(1), -p(0),   0.0 };

        // transform Pluecker line representation to local CT frames
        const auto L1 = M1 * L * M1transp;
        const auto L2 = M2 * L * M2transp;

        // 2D radon parameters of the lines
        const auto line1 = plueckerTo2DRadon(L1, originRadonTransp);
        const auto line2 = plueckerTo2DRadon(L2, originRadonTransp);

        // boundary checks and append coordinates
        if(intersectsDetector(line1) && intersectsDetector(line2))
        {
            lineSet1.push_back(line1);
            lineSet2.push_back(line2);
        }
    }

    return std::make_pair(std::move(lineSet1), std::move(lineSet2));
}

std::pair<IntermedGen2D2D::LineSet, IntermedGen2D2D::LineSet>
IntermedGen2D2D::linePairs(const mat::ProjectionMatrix& P1, const mat::ProjectionMatrix& P2,
                           const Chunk2D<float>::Dimensions& projSize) const
{
    const auto originRadon2D = 0.5 * (mat::Matrix<2, 1>(projSize.width, projSize.height) -
                                      mat::Matrix<2, 1>(1.0));
    return linePairs(P1, P2, projSize, originRadon2D, _angleIncrement);
}

Radon2DCoord IntermedGen2D2D::plueckerTo2DRadon(const mat::Matrix<3, 3>& L,
                                                const mat::Matrix<1, 2>& originRadon)
{
    // detector lines `l` in homogeneous coordinates
    mat::Matrix<3, 1> l{ L.get<1, 2>(), L.get<2, 0>(), L.get<0, 1>() };

    // normalize with length of line normal to obtain [nx, ny, -s]
    l /= l.subMat<0, 1>().norm();

    // radon parameter distance `s` and angle `mu`
    const auto mu = std::atan2(l.get<1>(), l.get<0>());
    const auto s = -l.get<2>() - originRadon * l.subMat<0, 1>();

    return { float(mu), float(s) };
}

// #### IntermedGen2D3D ####
// -----------------------------

float IntermedGen2D3D::lineDistance() const { return _lineDistance; }

void IntermedGen2D3D::setLineDistance(float lineDistance)
{
    if(qFuzzyIsNull(lineDistance))
        throw std::domain_error("IntermedGen2D3D::setLineDistance: line distance is close to zero");
    if(std::abs(lineDistance) < 1.0f)
        qWarning("Line distance below 1 is not meaningful, due to underlying linear interpolation");
    if(lineDistance < 0.0f)
        qWarning("Negative sign of the line distance is ignored");

    _lineDistance = std::abs(lineDistance);
}

const std::vector<Radon3DCoord>& IntermedGen2D3D::lastSampling() const { return _lastSampling; }

IntermediateFctPair IntermedGen2D3D::intermedFctPair(const Chunk2D<float>& proj,
                                                     const mat::ProjectionMatrix& P,
                                                     const VoxelVolume<float>& volume,
                                                     float plusMinusH_mm,
                                                     imgproc::DiffMethod derivativeMethodProj)
{
    mat::Matrix<2, 1> projSize(proj.width(), proj.height());
    auto imgDiag = static_cast<float>(projSize.norm());

    const auto nbS  = uint(ceil(imgDiag / _lineDistance));
    const auto nbMu = uint(ceil(double(nbS) * PI_2));

    SamplingRange sRange{ -0.5f * imgDiag, 0.5f * imgDiag };
    SamplingRange muRange{ float(0.0_deg), float(180.0_deg) };

    qDebug("Sample intermediate function with: %i x %i samples.", nbS, nbMu);

    // compute intermediate function from projections
    IntermediateProj intermedFctOfProj(proj, P.intrinsicMatK());
    auto intermProj = intermedFctOfProj.sampled(muRange, nbMu, sRange, nbS,
                                                derivativeMethodProj).data();

    // compute intermediate function from volume
    if(_useSubsampling)
    {
        uint seed = std::random_device{}(); // pull random seed for subsampling
        intermProj = randomSubset(std::move(intermProj), seed, _subsampleLevel);
        _lastSampling = randomSubset(intersectionPlanesWCS(muRange.linspace(nbMu),
                                                           sRange.linspace(nbS),
                                                           P,
                                                           intermedFctOfProj.origin()),
                                     seed, _subsampleLevel);
    }
    else
        _lastSampling = intersectionPlanesWCS(muRange.linspace(nbMu),
                                              sRange.linspace(nbS),
                                              P,
                                              intermedFctOfProj.origin());
    IntermediateVol intermedFctOfVol(volume);
    auto intermVol = intermedFctOfVol.sampled(_lastSampling, plusMinusH_mm);

    return IntermediateFctPair(intermProj, intermVol, IntermediateFctPair::VolumeDomain);
}

IntermediateFctPair IntermedGen2D3D::intermedFctPair(const Chunk2D<float>& proj,
                                                     const mat::ProjectionMatrix& P,
                                                     const OCL::VolumeResampler& radon3dSampler,
                                                     imgproc::DiffMethod derivativeMethodProj)
{
    return intermedFctPair(proj, P, radon3dSampler,
                           static_cast<imgproc::FiltMethod>(derivativeMethodProj));
}

IntermediateFctPair IntermedGen2D3D::intermedFctPair(const Chunk2D<float>& proj,
                                                     const mat::ProjectionMatrix& P,
                                                     const VolumeResampler& radon3dSampler,
                                                     imgproc::FiltMethod filterMethodProj)
{
    mat::Matrix<2, 1> projSize(proj.width(), proj.height());
    auto imgDiag = static_cast<float>(projSize.norm());

    const auto nbS  = uint(ceil(imgDiag / _lineDistance));
    const auto nbMu = uint(ceil(double(nbS) * PI_2));

    SamplingRange sRange{ -0.5f * imgDiag, 0.5f * imgDiag };
    SamplingRange muRange{ float(0.0_deg), float(180.0_deg) };

    // compute intermediate function from projections
    IntermediateProj intermedFctOfProj(proj, P.intrinsicMatK());
    auto intermProj = intermedFctOfProj.sampled(muRange, nbMu, sRange, nbS,
                                                filterMethodProj).data();

    // compute intermediate function from volume
    if(_useSubsampling)
    {
        uint seed = std::random_device{}(); // pull random seed for subsampling
        intermProj = randomSubset(std::move(intermProj), seed, _subsampleLevel);
        _lastSampling = randomSubset(intersectionPlanesWCS(muRange.linspace(nbMu),
                                                           sRange.linspace(nbS),
                                                           P,
                                                           intermedFctOfProj.origin()),
                                     seed, _subsampleLevel);
    }
    else
        _lastSampling = intersectionPlanesWCS(muRange.linspace(nbMu),
                                              sRange.linspace(nbS),
                                              P,
                                              intermedFctOfProj.origin());

    auto intermVol = radon3dSampler.sample(toGeneric3DCoord(_lastSampling));

    return IntermediateFctPair(std::move(intermProj), std::move(intermVol),
                               IntermediateFctPair::VolumeDomain);
}

IntermediateFctPair IntermedGen2D3D::intermedFctPair(const OCL::ImageResampler& radon2dSampler,
                                                     const mat::ProjectionMatrix& P,
                                                     const Chunk2D<float>::Dimensions& projSize,
                                                     const OCL::VolumeResampler& radon3dSampler)
{
    const auto imgDiag = mat::Matrix<2, 1>(projSize.width, projSize.height).norm();
    const auto nbS  = uint(ceil(imgDiag / _lineDistance));
    const auto nbMu = uint(ceil(double(nbS) * PI_2));
    const mat::Matrix<2, 1> orig{ (projSize.width - 1) * 0.5, (projSize.height - 1) * 0.5 };
    const SamplingRange sRange{ -0.5f * float(imgDiag), 0.5f * float(imgDiag) };
    const SamplingRange muRange{ float(0.0_deg), float(180.0_deg) };

    const auto muSamples = muRange.linspace(nbMu);
    const auto sSamples = sRange.linspace(nbS);
    std::vector<Generic2DCoord> radon2DSamples;
    radon2DSamples.reserve(nbMu * nbS);
    for(auto s : sSamples)
        for(auto mu : muSamples)
            radon2DSamples.emplace_back(mu, s);

    // compute intermediate function from volume
    if(_useSubsampling)
    {
        const uint seed = std::random_device{}(); // pull random seed for subsampling
        radon2DSamples = randomSubset(std::move(radon2DSamples), seed, _subsampleLevel);
        _lastSampling = randomSubset(intersectionPlanesWCS(muSamples, sSamples, P, orig),
                                     seed, _subsampleLevel);
    }
    else
        _lastSampling = intersectionPlanesWCS(muSamples, sSamples, P, orig);

    auto intermProj = radon2dSampler.sample(radon2DSamples);
    auto intermVol = radon3dSampler.sample(toGeneric3DCoord(_lastSampling));

    return IntermediateFctPair(std::move(intermProj), std::move(intermVol),
                               IntermediateFctPair::VolumeDomain);
}

float IntermedGen2D3D::subsampleLevel() const
{
    return _subsampleLevel;
}

void IntermedGen2D3D::setSubsampleLevel(float subsampleLevel)
{
    if(subsampleLevel <= 0.0f)
        qCritical("New subsampling level ignored, since it is negative or zero.");
    else if(subsampleLevel > 1.0f)
        qCritical("New subsampling level ignored, since it is greater than one.");
    else
    {
        _subsampleLevel = subsampleLevel;
        _useSubsampling = true;
    }
}

void IntermedGen2D3D::toggleSubsampling(bool enabled)
{
    _useSubsampling = enabled;
}

 /*!
  * Constructs a vector that has a list of Radon3DCoord for all combinations of the 2D Radon line
  * coordinates `mu` (the angle) and `dist` (aka 's'), which is stored in `mu`-major order, i.e.
  * first all `mu` with the first `dist`, then all `mu` with the second `dist` etc. The 3D Radon
  * coordinates for a plane are determined by a projection matrix (plane must contain the source
  * position). The `origin` specifies the placement of the coordinate frame where `mu` and `dist`
  * are defined.
  */
std::vector<Radon3DCoord>
IntermedGen2D3D::intersectionPlanesWCS(const std::vector<float>& mu,
                                       const std::vector<float>& dist,
                                       const mat::ProjectionMatrix& P,
                                       const mat::Matrix<2, 1>& origin) const
{
    std::vector<Radon3DCoord> ret;
    ret.reserve(mu.size() * dist.size());

    mat::Matrix<2, 1> n2D;
    mat::Matrix<3, 1> n3D;
    const auto Mtransp = P.M().transposed();
    const auto srcPosTransp = P.sourcePosition().transposed();
    const auto originTransp = origin.transposed();

    Radon3DCoord coordWCS;
    for(const auto& s : dist)
        for(const auto& angle : mu)
        {
            n2D = { std::cos(angle), std::sin(angle) };
            const auto z = s + originTransp * n2D;
            n3D = Mtransp * vertcat(n2D, mat::Matrix<1, 1>{ -z });
            n3D.normalize();

            coordWCS.azimuth() = std::atan2(float(n3D.get<1>()), float(n3D.get<0>()));
            coordWCS.polar() = std::acos(float(n3D.get<2>()));
            coordWCS.dist() = float(srcPosTransp * n3D);

            ret.push_back(coordWCS);
        }

    return ret;
}

// #### IntermediateProj ####
// --------------------------

IntermediateProj::IntermediateProj(const Chunk2D<float>& proj, const mat::Matrix<3, 3>& K, bool useWeighting)
    : _intrinsicK(K)
    , _useWeighting(useWeighting)
{
    // perform cosine pre-weighting of projections
    if(_useWeighting)
    {
        Chunk2D<float> projCpy(proj);
        imgproc::cosWeighting(projCpy, K);

        // construct Radon transform
        _radon2D.reset(new OCL::RadonTransform2D(projCpy));
    }
    else
        _radon2D.reset(new OCL::RadonTransform2D(proj));
}

IntermediateProj::IntermediateProj(const Chunk2D<float> &proj) // Aichert approximation (no weighting)
    : IntermediateProj(proj, mat::Matrix<3,3>(), false)
{
}

OCL::ImageResampler IntermediateProj::sampler(const SamplingRange& angleRange, uint nbAngles,
                                              const SamplingRange& distRange, uint nbDist,
                                              imgproc::DiffMethod derivativeMethod) const
{
    const auto intermedFct = sampled(angleRange, nbAngles, distRange, nbDist, derivativeMethod);
    // construct and return resampler
    return OCL::ImageResampler(intermedFct, angleRange, distRange);
}

ImageResampler IntermediateProj::sampler(const SamplingRange& angleRange, uint nbAngles,
                                         const SamplingRange& distRange, uint nbDist,
                                         imgproc::FiltMethod filterMethod) const
{
    const auto intermedFct = sampled(angleRange, nbAngles, distRange, nbDist, filterMethod);
    // construct and return resampler
    return OCL::ImageResampler(intermedFct, angleRange, distRange);
}

Chunk2D<float> IntermediateProj::sampled(const SamplingRange& angleRange, uint nbAngles,
                                         const SamplingRange& distRange, uint nbDist,
                                         imgproc::DiffMethod derivativeMethod) const
{
    return sampled(angleRange, nbAngles, distRange, nbDist,
                   static_cast<imgproc::FiltMethod>(derivativeMethod));
}

Chunk2D<float> IntermediateProj::sampled(const SamplingRange &angleRange, uint nbAngles,
                                         const SamplingRange &distRange, uint nbDist,
                                         imgproc::FiltMethod filterMethod) const
{
    if(nbDist < 2)
        throw std::runtime_error("IntermediateProj::sampler: nbDist must be greater than 1.");

    const auto angleSamples = angleRange.linspace(nbAngles);
    const auto distSamples  = distRange.linspace(nbDist);

    // compute 2D Radon transform
    auto radonTransf = _radon2D->sampleTransform(angleSamples, distSamples);

    // compute filter (or partial derivative) along distance dimension
    imgproc::filter<1>(radonTransf, filterMethod);
    radonTransf /= (distSamples[1] - distSamples[0]);

    // perform post-weighting
    if(_useWeighting)
        postWeighting(radonTransf, angleSamples, distSamples);

    return radonTransf;
}

std::vector<float> IntermediateProj::sampled(const std::vector<Radon2DCoord>& samplingPts,
                                             float plusMinusH) const
{
    // sampling points for central difference
    std::vector<Radon2DCoord> samplingPtsDerivative;
    samplingPtsDerivative.reserve(2 * samplingPts.size());
    for(const auto& centralCoord : samplingPts)
    {
        samplingPtsDerivative.emplace_back(centralCoord.angle(), centralCoord.dist() - plusMinusH);
        samplingPtsDerivative.emplace_back(centralCoord.angle(), centralCoord.dist() + plusMinusH);
    }

    // line integrals
    auto lineIntegrals = _radon2D->sampleTransform(samplingPtsDerivative);
    auto* lineItegralPtr = lineIntegrals.data();

    // derivative
    std::vector<float> ret(samplingPts.size());
    for(auto& val : ret)
    {
        // factor 1/(2h) for central difference
        val = (lineItegralPtr[1] - lineItegralPtr[0]) / (2.0f * plusMinusH);
        lineItegralPtr += 2;
    }

    if(_useWeighting)
        postWeighting(ret, samplingPts);

    return ret;
}

void IntermediateProj::postWeighting(Chunk2D<float>& radonTransDerivative,
                                     const std::vector<float>& theta,
                                     const std::vector<float>& s) const
{
    const auto& K = _intrinsicK;
    const auto p = mat::Matrix<2, 1>{ K.get<0, 2>(), K.get<1, 2>() };
    const auto originShiftTransp = (p - _radon2D->origin()).transposed();
    const auto sSize = s.size();
    const auto tSize = theta.size();

    for(uint tIdx = 0; tIdx < tSize; ++tIdx)
    {
        mat::Matrix<2, 1> n = { std::cos(theta[tIdx]), std::sin(theta[tIdx]) };
        auto sCorr = originShiftTransp * n;

        for(uint sIdx = 0; sIdx < sSize; ++sIdx)
        {
            auto cosPlaneAnlge = cosineOfPlaneAngle((s[sIdx] - sCorr) * n + p, K);
            radonTransDerivative(tIdx, sIdx) /= float(std::pow(cosPlaneAnlge, 2.0));
        }
    }
}

void IntermediateProj::postWeighting(std::vector<float>& lineIntegralDerivative,
                                     const std::vector<Radon2DCoord>& samplingPts) const
{
    const auto& K = _intrinsicK;
    const auto p = mat::Matrix<2, 1>{ K.get<0, 2>(), K.get<1, 2>() };
    const auto originShiftTransp = (p - _radon2D->origin()).transposed();

    std::transform(lineIntegralDerivative.cbegin(), lineIntegralDerivative.cend(), // input 1
                   samplingPts.cbegin(),                                           // input 2
                   lineIntegralDerivative.begin(),                                 // output
                   [&K, &p, &originShiftTransp](float val, const Radon2DCoord& coord)
                   {
                       mat::Matrix<2, 1> n{ std::cos(coord.angle()), std::sin(coord.angle()) };
                       auto sCorr = originShiftTransp * n;
                       auto cosPlaneAnlge = cosineOfPlaneAngle((coord.dist() - sCorr) * n + p, K);
                       return val / float(std::pow(cosPlaneAnlge, 2.0));
                   });
}

double IntermediateProj::cosineOfPlaneAngle(const mat::Matrix<2, 1>& x, const Matrix3x3 K)
{
    // cosine of an angle between the vector pointing from the source to a certain pixel location
    // on the detector and the z-axis (principal ray) of the CT cooradinate frame (CTS):
    // 1. calculate direction d = K^-1 * [x1,x2,1]^t
    // 2. normalize and
    // 3. take 3rd component

    // back substitution to find 'd' in K*d = [x,y,1]^t
    mat::Matrix<3, 1> d;
    d.get<2>() = 1.0;
    d.get<1>() = (x.get<1>() - K.get<1, 2>()) / K.get<1, 1>();
    d.get<0>() = (x.get<0>() - d.get<1>() * K.get<0, 1>() - K.get<0, 2>()) / K.get<0, 0>();

    // cosine to z-axis = <unitDirection, [0 0 1]^t>
    return d.get<2>() / d.norm();
}

void IntermediateProj::setOrigin(float x, float y)
{
    _radon2D->setOrigin(x, y);
}

mat::Matrix<2, 1> IntermediateProj::origin() const
{
    return _radon2D->origin();
}


// #### IntermediateVol ####
// -----------------------------

IntermediateVol::IntermediateVol(const VoxelVolume<float> &vol)
    : _radon3D(vol)
{
}

OCL::VolumeResampler IntermediateVol::sampler(const SamplingRange& phiRange, uint nbPhi,
                                              const SamplingRange& thetaRange, uint nbTheta,
                                              const SamplingRange& distRange, uint nbDist,
                                              imgproc::DiffMethod derivativeMethod) const
{
    // compute 3D Radon transform
    auto radonTransf = _radon3D.sampleTransform(phiRange.linspace(nbPhi),
                                                thetaRange.linspace(nbTheta),
                                                distRange.linspace(nbDist));

    // compute (partial) derivative along distance dimension
    imgproc::diff<2>(radonTransf, derivativeMethod);

    // construct and return resampler
    return OCL::VolumeResampler(radonTransf, phiRange, thetaRange, distRange);
}

VolumeResampler IntermediateVol::sampler(const SamplingRange& phiRange, uint nbPhi,
                                         const SamplingRange& thetaRange, uint nbTheta,
                                         const SamplingRange& distRange, uint nbDist,
                                         imgproc::FiltMethod filterMethod) const
{
    // compute 3D Radon transform
    auto radonTransf = _radon3D.sampleTransform(phiRange.linspace(nbPhi),
                                                thetaRange.linspace(nbTheta),
                                                distRange.linspace(nbDist));

    // compute (partial) derivative along distance dimension
    imgproc::filter<2>(radonTransf, filterMethod);

    // construct and return resampler
    return OCL::VolumeResampler(radonTransf, phiRange, thetaRange, distRange);
}

std::vector<float> IntermediateVol::sampled(const std::vector<Radon3DCoord>& samplingPointsWCS,
                                            float plusMinusH_mm) const
{
    std::vector<float> ret;
    ret.reserve(samplingPointsWCS.size());

    for(const auto& smpl : samplingPointsWCS)
    {
        // compute two plane integrals at points dist +- h (central difference)
        auto value = _radon3D.planeIntegral(smpl.azimuth(), smpl.polar(), smpl.dist() + plusMinusH_mm )
                     - _radon3D.planeIntegral(smpl.azimuth(), smpl.polar(), smpl.dist() - plusMinusH_mm );

        ret.push_back(value / (2.0f * plusMinusH_mm)); // factor 1/(2h) for central difference
    }

    return ret;
}


// #### Radon3DCoordTransform ####
// -----------------------------

Radon3DCoordTransform::Radon3DCoordTransform(size_t nbCoords, uint oclDeviceNb)
    : _q(OpenCLConfig::instance().context(), OpenCLConfig::instance().devices()[oclDeviceNb])
    , _homTransfBuf(16, _q)
    , _initialPlanesRadonCoord(nbCoords * 3, _q)
    , _initialPlanesHomCoord(OpenCLConfig::instance().context(), CL_MEM_READ_WRITE,
                             nbCoords * 4 * sizeof(float))
    , _transformedCoords(OpenCLConfig::instance().context(), CL_MEM_READ_WRITE,
                         nbCoords * 3 * sizeof(float))
{
}

Radon3DCoordTransform::Radon3DCoordTransform(const std::vector<Radon3DCoord>& initialCoords,
                                             uint oclDeviceNb)
    : Radon3DCoordTransform(initialCoords.size(), oclDeviceNb)

{
    addKernels();

    _initialPlanesRadonCoord.writeToDev(&initialCoords.front().coord1());
    transformRadonToHom();
}

Radon3DCoordTransform::Radon3DCoordTransform(const std::vector<HomCoordPlaneNormalized>& initialCoords, uint oclDeviceNb)
    : Radon3DCoordTransform(initialCoords.size(), oclDeviceNb)
{
    addKernels();
    _q.enqueueWriteBuffer(_initialPlanesHomCoord, CL_TRUE, 0,
                          initialCoords.size() * 4 * sizeof(float), initialCoords.data());
}

void Radon3DCoordTransform::resetIninitialCoords(const std::vector<Radon3DCoord>& initialCoords)
{
    if(nbCoords() != initialCoords.size())
        recreateBuffers(initialCoords.size());

    _initialPlanesRadonCoord.writeToDev(&initialCoords.front().coord1());
    transformRadonToHom();
}

void Radon3DCoordTransform::resetIninitialCoords(const std::vector<HomCoordPlaneNormalized>& initialCoords)
{
    if(nbCoords() != initialCoords.size())
        recreateBuffers(initialCoords.size());

    _q.enqueueWriteBuffer(_initialPlanesHomCoord, CL_TRUE, 0,
                          initialCoords.size() * 4 * sizeof(float), initialCoords.data());
}

const cl::Buffer& Radon3DCoordTransform::transform(const Homography3D& homography) const
{
    const auto H = homography.transposed();

    std::transform(H.begin(), H.end(), _homTransfBuf.hostPtr(),
                   [](double val){ return float(val); });
    _homTransfBuf.transferPinnedMemToDev(false);

    auto kernel = OpenCLConfig::instance().kernel(CL_KERNEL_HOM2RADON);
    kernel->setArg(0, _homTransfBuf.devBuffer());
    kernel->setArg(1, _initialPlanesHomCoord);
    kernel->setArg(2, _transformedCoords);

    cl::Event event;
    _q.enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(nbCoords()), cl::NullRange, nullptr,
                            &event);
    // wait for buffer is ready
    event.wait();

    return _transformedCoords;
}

const cl::Buffer& Radon3DCoordTransform::transform(const Matrix3x3& rotation,
                                                   const Vector3x1& translation) const
{
    return transform( Homography3D{ rotation, translation } );
}

std::vector<Radon3DCoord>
Radon3DCoordTransform::transformedCoords(const Matrix3x3& rotation,
                                         const Vector3x1& translation) const
{
    std::vector<Radon3DCoord> ret;

    transform(rotation, translation);

    auto nbCoords = this->nbCoords();
    ret.resize(nbCoords);
    _q.enqueueReadBuffer(_transformedCoords, CL_TRUE, 0, nbCoords * 3 * sizeof(float), ret.data());

    return ret;
}

std::vector<HomCoordPlaneNormalized> Radon3DCoordTransform::initialHomCoords() const
{
    std::vector<HomCoordPlaneNormalized> ret(nbCoords());

    _q.enqueueReadBuffer(_initialPlanesHomCoord, CL_TRUE, 0, nbCoords() * 4 * sizeof(float), ret.data());

    return ret;
}

void Radon3DCoordTransform::addKernels() const
{
    OCL::ClFileLoader clFileLoader;

    clFileLoader.setFileName("processing/" + CL_KERNEL_HOM2RADON + ".cl");
    OpenCLConfig::instance().addKernel(CL_KERNEL_HOM2RADON, clFileLoader.loadSourceCode());

    clFileLoader.setFileName("processing/" + CL_KERNEL_RADON2HOM + ".cl");
    OpenCLConfig::instance().addKernel(CL_KERNEL_RADON2HOM, clFileLoader.loadSourceCode());
}

size_t Radon3DCoordTransform::nbCoords() const
{
    return _initialPlanesRadonCoord.nbElements() / 3;
}

void Radon3DCoordTransform::recreateBuffers(size_t nbCoords)
{
    _initialPlanesRadonCoord = PinnedBufHostWrite<float>(nbCoords * 3, _q);
    _initialPlanesHomCoord = cl::Buffer(OpenCLConfig::instance().context(), CL_MEM_READ_WRITE,
                                        nbCoords * 4 * sizeof(float));
    _transformedCoords = cl::Buffer(OpenCLConfig::instance().context(), CL_MEM_READ_WRITE,
                                    nbCoords * 3 * sizeof(float));
}

void Radon3DCoordTransform::transformRadonToHom() const
{
    auto kernel = OpenCLConfig::instance().kernel(CL_KERNEL_RADON2HOM);

    kernel->setArg(0, _initialPlanesRadonCoord.devBuffer());
    kernel->setArg(1, _initialPlanesHomCoord);

    _q.enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(_initialPlanesRadonCoord.nbElements() / 3));
}

} // namespace OCL
} // namespace CTL

namespace {

template<class T>
std::vector<T> randomSubset(std::vector<T>&& fullSamples, uint seed, float subsampleLevel)
{
    auto newNbElements = uint(std::ceil(subsampleLevel * fullSamples.size()));
    std::vector<T> ret(newNbElements);

    std::mt19937 rng;
    rng.seed(seed);

    std::vector<uint> indices(fullSamples.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::shuffle(indices.begin(), indices.end(), rng);

    indices.resize(newNbElements);
    std::sort(indices.begin(), indices.end());

    for(uint smpl = 0; smpl < newNbElements; ++smpl)
        ret[smpl] = fullSamples[indices[smpl]];

    return ret;
}

} // unnamed namespace
