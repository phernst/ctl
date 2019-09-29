#include "radontransform3d.h"
#include "mat/matrix_utils.h"
#include "ocl/clfileloader.h"
#include <QDebug>

const uint PATCH_SIZE = 16; //!< size of a patch that is computed within an OpenCL workgroup
const std::string CL_FILE_NAME = "processing/planeIntegral.cl"; //!< path to .cl file
const std::string CL_KERNEL_NAME = "planeInt"; //!< name of the OpenCL kernel function
const std::string CL_PROGRAM_NAME = "planeIntegral"; //!< OCL program name

namespace CTL {
namespace OCL {

/*!
 * Creates a RadonTransform3D instance. The data in \a volume will be transfered to all available
 * OpenCL devices (for multi-GPU use) immediately (stored internally as cl::Image3D).
 */
RadonTransform3D::RadonTransform3D(const VoxelVolume<float> &volume)
    : _p{ sliceDim(volume.dimensions()),
          volume.smallestVoxelSize(),
          volume.dimensions(),
          volume.offset(),
          volume.voxelSize() }
{
    // check for valid OpenCLConfig
    if(!OpenCLConfig::instance().isValid())
        throw std::runtime_error("OpenCLConfig has not been initiated");

    // add load and add kernel source code
    OCL::ClFileLoader clFile(CL_FILE_NAME);
    if(!clFile.isValid())
        throw std::runtime_error(CL_FILE_NAME + "\nis not readable");
    const auto clSourceCode = clFile.loadSourceCode();
    OpenCLConfig::instance().addKernel(CL_KERNEL_NAME, clSourceCode, CL_PROGRAM_NAME);

    // initialize task for each device
    auto nbDevices = OpenCLConfig::instance().devices().size();
    _tasks.reserve(nbDevices);
    for(uint devNb = 0; devNb < nbDevices; ++devNb)
        _tasks.emplace_back(volume, _p, devNb);
}

/*!
 * Returns the 3D Radon transform of volume data for the set of sampling points given by
 * \a azimuthAngleSampling, \a polarAngleSampling and \a distanceSampling.
 *
 * This will return the plane integral for all combinations of angles and distances passed,
 * i.e. \a azimuthAngleSampling, \a polarAngleSampling and \a distanceSampling define the grid
 * of the returned Radon transform.
 *
 * This method supports multi-GPU and will automatically split the task (w.r.t. to the distance
 * samples) across all available devices (as returned by OpenCLConfig::instance().devices() when
 * this instance of RadonTransform3D had been created).
 */
VoxelVolume<float> RadonTransform3D::sampleTransform(const std::vector<float>& azimuthAngleSampling,
                                                     const std::vector<float>& polarAngleSampling,
                                                     const std::vector<float>& distanceSampling) const
{
    if(azimuthAngleSampling.size() > UINT_MAX &&
       polarAngleSampling.size() > UINT_MAX &&
       distanceSampling.size() > UINT_MAX)
    {
        qCritical() << "RadonTransform3D::sampleTransform: number of requested samples exceeds uint";
        return { 0, 0, 0 };
    }

    const auto nbAziSmpl = uint(azimuthAngleSampling.size());
    const auto nbPolSmpl = uint(polarAngleSampling.size());
    const auto nbDistSmpl = uint(distanceSampling.size());

    VoxelVolume<float> ret(nbAziSmpl, nbPolSmpl, nbDistSmpl);
    ret.allocateMemory();

    const uint nbPatches = _p.nbPatches();

    auto writeToRet = [&ret, nbPatches, nbDistSmpl] (const DeviceResult& devRes, uint azi, uint pol)
    {
        auto devPtr = devRes.first;
        float sum;
        uint patchIdx;

        // wait for device
        devRes.second->wait();

        // add up all partial sums (patches) for each distance
        for(uint dist = 0; dist < nbDistSmpl; ++dist)
        {
            sum = 0.0f;
            for(patchIdx = 0; patchIdx < nbPatches; ++patchIdx)
            {
                sum += *devPtr;
                ++devPtr;
            }
            ret(azi, pol, dist) = sum;
        }
    };

    auto nbDevices = _tasks.size();
    std::vector<DeviceResult> devResults(nbDevices);
    std::vector<bool> devRunning(nbDevices, false);
    std::vector<uint> aziAngle(nbDevices);
    std::vector<uint> polAngle(nbDevices);
    uint azi, pol, dev = 0;

    try{
        for(auto& task : _tasks)
            task.makeBufs(distanceSampling);

        for(pol = 0; pol < nbPolSmpl; ++pol)
            for(azi = 0; azi < nbAziSmpl; ++azi)
            {
                if(devRunning[dev])
                {
                    // wait for result and store in "ret"
                    writeToRet(devResults[dev], aziAngle[dev], polAngle[dev]);
                    devRunning[dev] = false;
                }

                // start async calculation on device
                devResults[dev] = _tasks[dev].planeIntegrals(azimuthAngleSampling[azi],
                                                             polarAngleSampling[pol]);
                // memory for device state
                aziAngle[dev] = azi;
                polAngle[dev] = pol;
                devRunning[dev] = true;

                ++dev;
                if(dev == nbDevices)
                    dev = 0;
             }

        // evaluate remaining running devices
        for(uint dev = 0; dev < nbDevices; ++dev)
            if(devRunning[dev])
                writeToRet(devResults[dev], aziAngle[dev], polAngle[dev]);

    } catch(const cl::Error& e)
    {
        qCritical() << "OpenCL error:" << e.what() << "(" << e.err() << ")";
    }

    // multiply result with area of a pixel
    return ret * std::pow(_p.reso, 2.0f);
}

/*!
 * Same as
 * sampleTransform(const std::vector<float>&, const std::vector<float>&, const std::vector<float>&),
 * but computes the 3D Radon transform on a equidistantly spaced grid, which is defined by sampling
 * ranges and number of samples in each direction.
 * Note that the returned VoxelVolume that contains the 3D Radon transform includes the voxel size
 * and volume offset according to the specified ranges and number of samples (i.e. spacing on the
 * grid and center of the ranges).
 */
VoxelVolume<float>
RadonTransform3D::sampleTransform(SamplingRange azimuthRange, uint nbAzimuthSamples,
                                  SamplingRange polarRange, uint nbPolarSamples,
                                  SamplingRange distanceRange, uint nbDistanceSamples) const
{
    auto ret = sampleTransform(azimuthRange.linspace(nbAzimuthSamples),
                               polarRange.linspace(nbPolarSamples),
                               distanceRange.linspace(nbDistanceSamples));

    ret.setVoxelSize(azimuthRange.spacing(nbAzimuthSamples),
                     polarRange.spacing(nbPolarSamples),
                     distanceRange.spacing(nbDistanceSamples));
    ret.setVolumeOffset(azimuthRange.center(), polarRange.center(), distanceRange.center());

    return ret;
}

/*!
 * Returns the plane integral of the volume data along the plane specified by its normal vector
 * \a planeUnitNormal and its distance from the origin \a planeDistanceFromOrigin.
 */
float RadonTransform3D::planeIntegral(const mat::Matrix<3, 1>& planeUnitNormal,
                                      double planeDistanceFromOrigin) const
{
    if(_tasks.empty())
    {
        qCritical() << "no OpenCL device initialized";
        return 0.0f;
    }

    const float* ptr;
    try{
        // use first device for computation
        _tasks[0].makeBufs(std::vector<float>{ float(planeDistanceFromOrigin) });
        const auto& res = _tasks[0].planeIntegrals(planeUnitNormal);
        res.second->wait();
        ptr = res.first;
    } catch(const cl::Error& e)
    {
        qCritical() << "OpenCL error:" << e.what() << "(" << e.err() << ")";
        return 0.0f;
    }

    // sum-up patch results
    float sum = 0.0f;
    for(uint val = 0; val < _p.nbPatches(); ++val)
        sum += ptr[val];

    return sum * std::pow(_p.reso, 2.0f);
}

/*!
 * Returns the plane integral of the volume data along the plane specified by its angles w.r.t. the
 * world coordinate system \a planeNormalAzimutAngle and \a planeNormalPolarAngle (polar
 * coordinates) as well as its distance from the origin \a planeDistanceFromOrigin.
 */
float RadonTransform3D::planeIntegral(double planeNormalAzimutAngle, double planeNormalPolarAngle,
                                      double planeDistanceFromOrigin) const
{
    Vector3x1 planeNormal{
        std::sin(planeNormalPolarAngle) * std::cos(planeNormalAzimutAngle),
        std::sin(planeNormalPolarAngle) * std::sin(planeNormalAzimutAngle),
        std::cos(planeNormalPolarAngle)
    };

    return planeIntegral(planeNormal, planeDistanceFromOrigin);
}

/*!
 * Sets the resolution (i.e. pixels size) for slices used to compute the plane integrals to
 * \a pixelResolution. Resolution is specified in millimeters.
 */
void RadonTransform3D::setSliceResolution(float pixelResolution)
{
    Q_ASSERT(pixelResolution > 0.0f);
    const float factor = _p.reso / pixelResolution;
    _p.reso = pixelResolution;

    // change number of pixels in slice
    const uint newNbPixel = nextMultipleOfN(_p.dim.width * factor, PATCH_SIZE);
    _p.dim = { newNbPixel, newNbPixel };

    for(auto& task : _tasks)
        task.sliceDimensionsChanged();
}

/*!
 * Returns the dimensions (i.e. number of pixels) of slices used to compute the plane integrals.
 */
Chunk2D<float>::Dimensions RadonTransform3D::sliceDimensions() const { return _p.dim; }

/*!
 * Returns the resolution (i.e. pixels size) of slices used to compute the plane integrals.
 */
float RadonTransform3D::sliceResolution() const { return _p.reso; }

/*!
 * Returns the dimensions (i.e. number of voxels) of the volume managed by this instance.
 */
const VoxelVolume<float>::Dimensions& RadonTransform3D::volDim() const { return _p.volDim; }

/*!
 * Returns the offset (in mm) of the volume managed by this instance.
 */
const VoxelVolume<float>::Offset& RadonTransform3D::volOffset() const { return _p.volOffset; }

/*!
 * Returns the size of the voxels in the volume managed by this instance.
 */
const VoxelVolume<float>::VoxelSize& RadonTransform3D::volVoxSize() const { return _p.volVoxSize; }

Chunk2D<float>::Dimensions
RadonTransform3D::sliceDim(const VoxelVolume<float>::Dimensions& volDim)
{
    auto sliceDim = std::max(std::max(volDim.x, volDim.y), volDim.z);

    sliceDim = nextMultipleOfN(std::ceil(1.4142f * sliceDim), PATCH_SIZE); // sqrt(2) times larger

    return { uint(sliceDim), uint(sliceDim) };
}

uint RadonTransform3D::Parameters::nbPatches() const
{
    return (dim.width / PATCH_SIZE) * (dim.height / PATCH_SIZE);
}

uint RadonTransform3D::nextMultipleOfN(uint value, uint N)
{
    uint ret = value;

    if(value % N != 0u)
        ret = uint((std::floor(double(value) / double(N)) + 1.0)) * N;

    return ret;
}

RadonTransform3D::SingleDevice::SingleDevice(const VoxelVolume<float>& volume,
                                             const Parameters& params, uint oclDeviceNb)
    : _p(params)
    , _volumeCorner{ volume.offset().x - 0.5 * volume.dimensions().x * volume.voxelSize().x,
                     volume.offset().y - 0.5 * volume.dimensions().y * volume.voxelSize().y,
                     volume.offset().z - 0.5 * volume.dimensions().z * volume.voxelSize().z }
    , _templatePlaneStart{ - _p.reso * 0.5 * (_p.dim.width - 1),
                           - _p.reso * 0.5 * (_p.dim.height - 1),
                           0.0 }
    , _q(OpenCLConfig::instance().context(), OpenCLConfig::instance().devices()[oclDeviceNb])
    , _homoBuf(1, _q)
    , _distShiftBuf(1, _q)
    , _volImage3D(OpenCLConfig::instance().context(),
                  CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                  cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                  volume.dimensions().x,
                  volume.dimensions().y,
                  volume.dimensions().z)
    , _kernel(nullptr)
    , _nbDist(0u)
{
    try
    {
        // Create kernel
        _kernel = OpenCLConfig::instance().kernel(CL_KERNEL_NAME, CL_PROGRAM_NAME);

        // write 3d image
        cl::size_t<3> volDim;
        volDim[0] = volume.dimensions().x;
        volDim[1] = volume.dimensions().y;
        volDim[2] = volume.dimensions().z;
        _q.enqueueWriteImage(_volImage3D, CL_TRUE, cl::size_t<3>(), volDim, 0, 0,
                             const_cast<float*>(volume.rawData()));

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    if(_kernel == nullptr)
        throw std::runtime_error("kernel pointer not valid");
}

RadonTransform3D::DeviceResult
RadonTransform3D::SingleDevice::planeIntegrals(const mat::Matrix<3, 1>& planeUnitNormal) const
{
    Q_ASSERT(qFuzzyCompare(planeUnitNormal.norm(), 1.0));

    if(_p.volVoxSize.x <= 0.0f || _p.volVoxSize.y <= 0.0f || _p.volVoxSize.z <= 0.0f)
        throw std::runtime_error("voxel size is zero or negative");

    // calculate homography that maps a XY-plane to the requested plane
    const auto H = transformXYPlaneToCentralPlane(planeUnitNormal);

    // write data to pinned memory (host side)
    *_distShiftBuf.hostPtr() = cl_float3{ { float(planeUnitNormal.get<0>()) / _p.volVoxSize.x,
                                            float(planeUnitNormal.get<1>()) / _p.volVoxSize.y,
                                            float(planeUnitNormal.get<2>()) / _p.volVoxSize.z }
                                        };

    *_homoBuf.hostPtr() = cl_float16{ { float(H(0,0)), float(H(0,1)), float(H(0,2)), float(H(0,3)),
                                        float(H(1,0)), float(H(1,1)), float(H(1,2)), float(H(1,3)),
                                        float(H(2,0)), float(H(2,1)), float(H(2,2)), float(H(2,3)) }
                                    };

    // transfer pinned memory contents to device
    _distShiftBuf.transferPinnedMemToDev(false);
    _homoBuf.transferPinnedMemToDev(false);

    // set kernel arguments and run
    _kernel->setArg(0, _homoBuf.devBuffer());
    _kernel->setArg(1, _distShiftBuf.devBuffer());
    _kernel->setArg(2, _distanceBuf);
    _kernel->setArg(3, _nbDist);
    _kernel->setArg(4, _resultBufAllDist->devBuffer());
    _kernel->setArg(5, _volImage3D);

    _q.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(_p.dim.width, _p.dim.height),
                                                     cl::NDRange(PATCH_SIZE ,PATCH_SIZE));

    // read result
    std::unique_ptr<cl::Event> readEvent(new cl::Event);
    _resultBufAllDist->transferDevToPinnedMem(false, readEvent.get());

    return { _resultBufAllDist->hostPtr(), std::move(readEvent) };
}

RadonTransform3D::DeviceResult RadonTransform3D::SingleDevice::planeIntegrals(double planeNormalAzimutAngle,
                                                                              double planeNormalPolarAngle) const
{
    Vector3x1 planeNormal{
        std::sin(planeNormalPolarAngle) * std::cos(planeNormalAzimutAngle),
        std::sin(planeNormalPolarAngle) * std::sin(planeNormalAzimutAngle),
        std::cos(planeNormalPolarAngle)
    };
    return planeIntegrals(planeNormal);
}

void RadonTransform3D::SingleDevice::makeBufs(const std::vector<float>& distanceSampling)
{
    if(distanceSampling.size() != _nbDist) // buffer size changes
    {
        _nbDist = uint(distanceSampling.size());
        _distanceBuf = cl::Buffer(OpenCLConfig::instance().context(), CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                                  _nbDist * sizeof(float));
        _resultBufAllDist.reset(new PinnedBufHostRead<float>(_nbDist * _p.nbPatches(), _q));
    }

    _q.enqueueWriteBuffer(_distanceBuf, CL_TRUE, 0, _nbDist * sizeof(float), distanceSampling.data());
}

void RadonTransform3D::SingleDevice::sliceDimensionsChanged()
{
    _resultBufAllDist.reset(new PinnedBufHostRead<float>(_nbDist * _p.nbPatches(), _q));

    _templatePlaneStart = { - _p.reso * 0.5 * (_p.dim.width - 1),
                            - _p.reso * 0.5 * (_p.dim.height - 1),
                            0.0 };
}

mat::Matrix<3, 3>
RadonTransform3D::SingleDevice::rotationXYPlaneToPlane(const mat::Matrix<3, 1>& n) const
{
    Vector3x1 r1, r2(0.0), r3{ n.get<0>(), n.get<1>(), n.get<2>() };

    // find axis that is as most as perpendicular to r3
    uint axis = std::abs(r3.get<0>()) < std::abs(r3.get<1>()) ? 0 : 1;
    axis = std::abs(r3(axis)) < std::abs(r3.get<2>()) ? axis : 2;
    r2(axis) = 1.0;
    r2 = mat::cross(r3, r2);
    r2 /= r2.norm();
    r1 = mat::cross(r2, r3);

    return mat::horzcat(mat::horzcat(r1, r2), r3);
}

mat::Matrix<3, 4>
RadonTransform3D::SingleDevice::transformXYPlaneToCentralPlane(const mat::Matrix<3, 1>& n) const
{
    const auto rotMatTransp = rotationXYPlaneToPlane(n);
    const auto translationVec = rotMatTransp * _templatePlaneStart - _volumeCorner;

    Matrix3x3 voxelSizeNormalization = mat::diag(
        Vector3x1{ 1.0 / _p.volVoxSize.x, 1.0 / _p.volVoxSize.y, 1.0 / _p.volVoxSize.z });

    return voxelSizeNormalization * mat::horzcat(_p.reso * rotMatTransp, translationVec);
}

} // namespace OCL

std::vector<Generic3DCoord> toGeneric3DCoord(const std::vector<Radon3DCoord>& radonCoord)
{
    std::vector<Generic3DCoord> ret(radonCoord.size());
    std::copy(radonCoord.cbegin(), radonCoord.cend(), ret.begin());
    return ret;
}

std::vector<HomCoordPlaneNormalized> toHomCoordPlane(const std::vector<Radon3DCoord>& radonCoord)
{
    auto ret = std::vector<HomCoordPlaneNormalized>(radonCoord.size());
    auto planePtr = ret.data();
    float sinPol, cosPol, sinAzi, cosAzi;

    for(const auto& coord : radonCoord)
    {
        sinPol = std::sin(coord.polar());
        cosPol = std::cos(coord.polar());
        sinAzi = std::sin(coord.azimuth());
        cosAzi = std::cos(coord.azimuth());
        planePtr->data[0] = sinPol * cosAzi;
        planePtr->data[1] = sinPol * sinAzi;
        planePtr->data[2] = cosPol;
        planePtr->data[3] = -coord.dist();

        ++planePtr;
    }

    return ret;
}

} // namespace CTL
