#ifndef CONSISTENCY_H
#define CONSISTENCY_H

#include "img/chunk2d.h"
#include "img/voxelvolume.h"
#include "mat/mat.h"
#include "processing/coordinates.h"
#include "processing/errormetrics.h"
#include "processing/imageprocessing.h" //<- to be renamed
#include "processing/imageresampler.h"
#include "processing/radontransform2d.h"
#include "processing/radontransform3d.h"
#include "processing/volumeresampler.h"

/*
 * This header introduces classes for applications of Grangeat data consistency conditions.
 */

namespace CTL {

/*!
 * \class IntermediateFctPair
 * \brief Holds a pair of two corresponding intermediate function 'signals' as `shared_ptr`s to
 * `vector`s. The first vector is associated with an intermediate function from a projection image,
 * while the second vector may be computed from a projection or a volume.
 */

class IntermediateFctPair
{
public:
    enum Type{ ProjectionDomain, VolumeDomain };

    IntermediateFctPair(std::vector<float> first, std::vector<float> second, Type secondType);
    IntermediateFctPair(std::shared_ptr<const std::vector<float>> first, std::vector<float> second,
                        Type secondType);
    IntermediateFctPair(std::vector<float> first, std::shared_ptr<const std::vector<float>> second,
                        Type secondType);
    IntermediateFctPair(std::shared_ptr<const std::vector<float>> first,
                        std::shared_ptr<const std::vector<float>> second, Type secondType);

    double inconsistency(const imgproc::AbstractErrorMetric& metric = metric::L2,
                         bool swapInput = false) const;

    bool isEmpty() const;

    const std::vector<float>& first() const;
    const std::vector<float>& second() const;

    const std::shared_ptr<const std::vector<float>>& ptrToFirst() const;
    const std::shared_ptr<const std::vector<float>>& ptrToSecond() const;

    Type firstType() const { return ProjectionDomain; }
    Type secondType() const { return _secondType; }

private:
    std::shared_ptr<const std::vector<float>> _first;
    std::shared_ptr<const std::vector<float>> _second;

    Type _secondType;
};

namespace OCL {

/*!
 * \class IntermedGen2D2D
 * \brief Generator class that produces intermediate function pairs from two 2D projection images.
 */

/*!
 * \class IntermedGen2D3D
 * \brief Generator class that produces intermediate function pairs from a 2D projection image and a
 * 3D volume.
 */

/*!
 * \class IntermediateProj
 * \brief Transforms projections to Grangeat's intermediate space.
 */

/*!
 * \class IntermediateVol
 * \brief Transforms volumes to Grangeat's intermediate space.
 */

/*!
 * \class Radon3DCoordTransform
 * \brief Helper class that transforms (spherical) 3D Radon coordinates under an Euclidian
 * transform of the coordinate frame.
 */

class IntermedGen2D2D
{
public:
    using Lines = std::vector<Radon2DCoord>;

    double angleIncrement() const;
    void setAngleIncrement(double angleIncrement);

    // on the fly
    IntermediateFctPair intermedFctPair(const Chunk2D<float>& proj1,
                                        const mat::ProjectionMatrix& P1,
                                        const Chunk2D<float>& proj2,
                                        const mat::ProjectionMatrix& P2,
                                        float plusMinusH = 1.0f);

    // precomputed (origin must be the default origin: [(X-1)/2, (Y-1)/2])
    IntermediateFctPair intermedFctPair(const OCL::ImageResampler& radon2dSampler1,
                                        const mat::ProjectionMatrix& P1,
                                        const OCL::ImageResampler& radon2dSampler2,
                                        const mat::ProjectionMatrix& P2);

    // compute corresponding line pairs; line pairs intersect the detector with `projSize`
    static std::pair<Lines, Lines> linePairs(const mat::ProjectionMatrix& P1,
                                             const mat::ProjectionMatrix& P2,
                                             const Chunk2D<float>::Dimensions& projSize,
                                             const mat::Matrix<2, 1>& originRadon,
                                             double angleIncrement = 0.01_deg);
    // `originRadon` defaults to (projSize-[1,1])/2
    static std::pair<Lines, Lines> linePairs(const mat::ProjectionMatrix& P1,
                                             const mat::ProjectionMatrix& P2,
                                             const Chunk2D<float>::Dimensions& projSize,
                                             double angleIncrement = 0.01_deg);

private:
    static double maxDistanceToCorners(const Chunk2D<float>::Dimensions& projSize,
                                       const mat::Matrix<2, 1>& originRadon);
    static mat::Matrix<3, 1> orthonormalTo(const mat::Matrix<3, 1>& v);
    static Radon2DCoord plueckerTo2DRadon(const mat::Matrix<3, 3>& L,
                                          const mat::Matrix<1, 2>& originRadon);

    double _angleIncrement = 0.01_deg; //!< increment rotation angle around the baseline
};

class IntermedGen2D3D
{
public:
    float accuracy() const;
    void setAccuracy(float accuracy);
    float subsampleLevel() const;
    void setSubsampleLevel(float subsampleLevel);
    void toggleSubsampling(bool enabled);
    imgproc::DiffMethod derivativeMethodInProjectionDomain() const;
    void setDerivativeMethodInProjectionDomain(const imgproc::DiffMethod& method);

    const std::vector<Radon3DCoord>& lastSampling() const;

    // fully on the fly
    IntermediateFctPair intermedFctPair(const Chunk2D<float>& proj,
                                        const mat::ProjectionMatrix& P,
                                        const VoxelVolume<float>& volume,
                                        float plusMinusH_mm);
    // projection on the fly
    IntermediateFctPair intermedFctPair(const Chunk2D<float>& proj,
                                        const mat::ProjectionMatrix& P,
                                        const OCL::VolumeResampler& radon3dSampler);

    // fully precomputed (origin must be the default origin: [(X-1)/2, (Y-1)/2])
    IntermediateFctPair intermedFctPair(const OCL::ImageResampler& radon2dSampler,
                                        const mat::ProjectionMatrix& P,
                                        const OCL::VolumeResampler& radon3dSampler);

private:
    float _accuracy = 1.0f;
    float _subsampleLevel = 1.0f;
    bool _useSubsampling = false;
    std::vector<Radon3DCoord> _lastSampling;
    imgproc::DiffMethod _derivativeMethod = imgproc::CentralDifference;

    std::vector<Radon3DCoord> intersectionPlanesWCS(const std::vector<float>& mu,
                                                    const std::vector<float>& dist,
                                                    const mat::ProjectionMatrix& P,
                                                    const mat::Matrix<2, 1>& origin) const;
    template<class T>
    std::vector<T> randomSubset(std::vector<T>&& fullSamples, uint seed) const;
};

class IntermediateProj
{
public:
    IntermediateProj(const Chunk2D<float>& proj, const mat::Matrix<3,3>& K, bool useWeighting = true);
    explicit IntermediateProj(const Chunk2D<float>& proj);

    void setDerivativeMethod(imgproc::DiffMethod method);
    void setOrigin(float x, float y);

    mat::Matrix<2, 1> origin() const;

    OCL::ImageResampler sampler(const SamplingRange& angleRange, uint nbAngles,
                                const SamplingRange& distRange, uint nbDist) const;

    Chunk2D<float> sampled(const SamplingRange& angleRange, uint nbAngles,
                           const SamplingRange& distRange, uint nbDist) const;
    std::vector<float> sampled(const std::vector<Radon2DCoord>& samplingPts,
                               float plusMinusH = 1.0f) const;

private:
    mat::Matrix<3, 3> _intrinsicK;
    std::unique_ptr<OCL::RadonTransform2D> _radon2D;
    imgproc::DiffMethod _derivativeMethod = imgproc::CentralDifference;
    bool _useWeighting;

    void postWeighting(Chunk2D<float>& radonTransDerivative,
                       const std::vector<float>& theta,
                       const std::vector<float>& s) const;
    void postWeighting(std::vector<float>& lineIntegralDerivative,
                       const std::vector<Radon2DCoord>& samplingPts) const;

    static double cosineOfPlaneAngle(const mat::Matrix<2, 1>& x, const Matrix3x3 K);
};

class IntermediateVol
{
public:
    explicit IntermediateVol(const VoxelVolume<float>& vol);

    OCL::VolumeResampler sampler(const SamplingRange& phiRange, uint nbPhi,
                                 const SamplingRange& thetaRange, uint nbTheta,
                                 const SamplingRange& distRange, uint nbDist,
                                 imgproc::DiffMethod method = imgproc::CentralDifference) const;

    std::vector<float> sampled(const std::vector<Radon3DCoord>& samplingPointsWCS, float plusMinusH_mm) const;


private:
    OCL::RadonTransform3D _radon3D;
};

class Radon3DCoordTransform
{
public:
    explicit Radon3DCoordTransform(const std::vector<Radon3DCoord>& initialCoords, uint oclDeviceNb = 0);
    explicit Radon3DCoordTransform(const std::vector<HomCoordPlaneNormalized>& initialCoords, uint oclDeviceNb = 0);

    void resetIninitialCoords(const std::vector<Radon3DCoord>& initialCoords);
    void resetIninitialCoords(const std::vector<HomCoordPlaneNormalized>& initialCoords);

    const cl::Buffer& transform(const Homography3D& homography) const;
    const cl::Buffer& transform(const Matrix3x3 &rotation, const Vector3x1 &translation) const;
    std::vector<Radon3DCoord> transformedCoords(const Matrix3x3 &rotation,
                                                const Vector3x1 &translation) const;

    std::vector<HomCoordPlaneNormalized> initialHomCoords() const;

private:
    Radon3DCoordTransform(size_t nbCoords, uint oclDeviceNb);

    cl::CommandQueue _q;
    PinnedBufHostWrite<float> _homTransfBuf;
    PinnedBufHostWrite<float> _initialPlanesRadonCoord;
    cl::Buffer _initialPlanesHomCoord;
    cl::Buffer _transformedCoords;

    void addKernels() const;
    size_t nbCoords() const;
    void recreateBuffers(size_t nbCoords);
    void transformRadonToHom() const;
};

} // namespace OCL
} // namespace CTL

#endif // CONSISTENCY_H
