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
 * Returns the 3D Radon transform of volume data for the set of sampling points given by
 * \a azimuthAngleSampling, \a polarAngleSampling, and \a distanceSampling.
 *
 * This will return the plane integral for all combinations of angles and distances passed.
 *
 * This method supports multi-GPU and will automatically split the task (w.r.t. to the distance
 * samples) across all available devices (as returned by OpenCLConfig::instance().devices() when
 * this instance of RadonTransform3D had been created).
 */
VoxelVolume<float> RadonTransform3D::sampleTransform(const std::vector<double>& azimuthAngleSampling,
                                                     const std::vector<double>& polarAngleSampling,
                                                     const std::vector<double>& distanceSampling) const
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

    auto writeToRet = [&ret, nbPatches] (const DeviceResult& devRes, uint azi, uint pol, uint dist)
    {
        devRes.second->wait();
        float sum = 0.0f;
        const auto ptr = devRes.first;
        for(uint val = 0; val < nbPatches; ++val)
            sum += ptr[val];

        ret(azi, pol, dist) = sum;
    };

    auto nbDevices = _tasks.size();
    std::vector<DeviceResult> devResults(nbDevices);
    std::vector<bool> devRunning(nbDevices, false);
    std::vector<uint> aziAngle(nbDevices);
    std::vector<uint> polAngle(nbDevices);
    std::vector<uint> distance(nbDevices);
    uint azi, pol, dist, dev = 0;

    for(dist = 0; dist < nbDistSmpl; ++dist)
        for(pol = 0; pol < nbPolSmpl; ++pol)
            for(azi = 0; azi < nbAziSmpl; ++azi)
            {
                if(devRunning[dev])
                {
                    // wait for result and store in "ret"
                    writeToRet(devResults[dev], aziAngle[dev], polAngle[dev], distance[dev]);
                    devRunning[dev] = false;
                }

                // start async calculation on device
                devResults[dev] = _tasks[dev].planeIntegral(azimuthAngleSampling[azi],
                                                            polarAngleSampling[pol],
                                                            distanceSampling[dist]);
                // memory for device state
                aziAngle[dev] = azi;
                polAngle[dev] = pol;
                distance[dev] = dist;
                devRunning[dev] = true;

                ++dev;
                if(dev == nbDevices)
                    dev = 0;
             }
    // evaluate remaining running devices
    for(uint dev = 0; dev < nbDevices; ++dev)
        if(devRunning[dev])
            writeToRet(devResults[dev], aziAngle[dev], polAngle[dev], distance[dev]);

    // multiply result with area of a pixel
    return ret * std::pow(_p.reso, 2.0f);
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
    // use first device for computation
    const auto& res = _tasks[0].planeIntegral(planeUnitNormal, planeDistanceFromOrigin);
    res.second->wait();
    float sum = 0.0f;
    const auto ptr = res.first;
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
    if(_tasks.empty())
    {
        qCritical() << "no OpenCL device initialized";
        return 0.0f;
    }
    // use first device for computation
    const auto& res = _tasks[0].planeIntegral(planeNormalAzimutAngle, planeNormalPolarAngle,
                                              planeDistanceFromOrigin);
    res.second->wait();
    float sum = 0.0f;
    const auto ptr = res.first;
    for(uint val = 0; val < _p.nbPatches(); ++val)
        sum += ptr[val];

    return sum * std::pow(_p.reso, 2.0f);
}

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
    return (dim.width/PATCH_SIZE)*(dim.height/PATCH_SIZE);
}

uint RadonTransform3D::nextMultipleOfN(uint value, uint N)
{
    uint ret = value;

    if(value % N != 0u)
        ret = uint((std::floor(double(value) / double(N)) + 1.0)) * N;

    return ret;
}

RadonTransform3D::SingleGPU::SingleGPU(const VoxelVolume<float>& volume, const Parameters& params,
                                       uint oclDeviceNb)
    : _p(params)
    , _q(OpenCLConfig::instance().context(), OpenCLConfig::instance().devices()[oclDeviceNb])
    , _volImage3D(OpenCLConfig::instance().context(),
                  CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                  cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                  volume.dimensions().x,
                  volume.dimensions().y,
                  volume.dimensions().z)
    , _homoBuf(OpenCLConfig::instance().context(),
               CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
               16 * sizeof(float))
    , _sliceDimBuf(OpenCLConfig::instance().context(),
                   CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                   2 * sizeof(uint))
    , _voxCornerBuf(OpenCLConfig::instance().context(),
                    CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                    3 * sizeof(float))
    , _resultBuf((_p.dim.width/PATCH_SIZE) * (_p.dim.height/PATCH_SIZE), _q)
{
    // Create kernel
    try
    {
        _kernel = OpenCLConfig::instance().kernel(CL_KERNEL_NAME, CL_PROGRAM_NAME);

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    if(_kernel == nullptr)
        throw std::runtime_error("kernel pointer not valid");


    // write buffers
    cl::size_t<3> volDim;
    volDim[0] = volume.dimensions().x;
    volDim[1] = volume.dimensions().y;
    volDim[2] = volume.dimensions().z;

    const cl_uint2 sliceDim{ _p.dim.width, _p.dim.height };

    const cl_float3 voxCorner{ -0.5f * (_p.volDim.x - 1) + _p.volOffset.x / _p.volVoxSize.x,
                               -0.5f * (_p.volDim.y - 1) + _p.volOffset.y / _p.volVoxSize.y,
                               -0.5f * (_p.volDim.z - 1) + _p.volOffset.z / _p.volVoxSize.z };

    _q.enqueueWriteImage(_volImage3D, CL_FALSE, cl::size_t<3>(), volDim, 0, 0,
                         const_cast<float*>(volume.rawData()));
    _q.enqueueWriteBuffer(_sliceDimBuf, CL_FALSE, 0, 2 * sizeof(uint), &sliceDim);
    _q.enqueueWriteBuffer(_voxCornerBuf, CL_FALSE, 0, 3 * sizeof(float), &voxCorner);
}

RadonTransform3D::DeviceResult RadonTransform3D::SingleGPU::planeIntegral(const mat::Matrix<3, 1>& planeUnitNormal,
                                                        double planeDistanceFromOrigin) const
{
    Q_ASSERT(qFuzzyCompare(planeUnitNormal.norm(), 1.0));

    std::unique_ptr<cl::Event> readEvent(new cl::Event);

    try {
        if(_p.volVoxSize.x <= 0.0f || _p.volVoxSize.y <= 0.0f || _p.volVoxSize.z <= 0.0f)
            throw std::runtime_error("voxel size is zero or negative");

        // calculate homography that maps a XY-plane to the requested plane
        const auto h = transformXYPlaneToPlane(
            mat::vertcat(planeUnitNormal, mat::Matrix<1, 1>(-planeDistanceFromOrigin)));

        // store in OpenCL specifiv vector format
        const cl_float16 h_cl{ float(h(0, 0)), float(h(0, 1)), float(h(0, 2)), float(h(0, 3)),
                               float(h(1, 0)), float(h(1, 1)), float(h(1, 2)), float(h(1, 3)),
                               float(h(2, 0)), float(h(2, 1)), float(h(2, 2)), float(h(2, 3)) };

        // write buffers
        _q.enqueueWriteBuffer(_homoBuf, CL_FALSE, 0, 12 * sizeof(float), &h_cl);

        // set kernel arguments and run
        _kernel->setArg(0, _voxCornerBuf);
        _kernel->setArg(1, _sliceDimBuf);
        _kernel->setArg(2, _homoBuf);
        _kernel->setArg(3, _resultBuf.devBuffer());
        _kernel->setArg(4, _volImage3D);

        _q.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(_p.dim.width, _p.dim.height),
                                                         cl::NDRange(PATCH_SIZE ,PATCH_SIZE));

        // read result
        _resultBuf.transferDevToPinnedMem(false, readEvent.get());
//        _q.enqueueReadBuffer(_resultBuf.devBuffer(), CL_FALSE, 0, nbPatches * sizeof(float), patchRes.data(),
//                             nullptr, readEvent.get());

    } catch(const cl::Error& e)
    {
        qCritical() << "OpenCL error:" << e.what() << "(" << e.err() << ")";
    } catch(const std::bad_alloc& e)
    {
        qCritical() << "allocation error:" << e.what();
    } catch(const std::exception& e)
    {
        qCritical() << "std exception:" << e.what();
    }

    return { _resultBuf.hostPtr(), std::move(readEvent) };
}

RadonTransform3D::DeviceResult RadonTransform3D::SingleGPU::planeIntegral(double planeNormalAzimutAngle,
                                                        double planeNormalPolarAngle,
                                                        double planeDistanceFromOrigin) const
{
    Vector3x1 planeNormal{
        std::sin(planeNormalPolarAngle) * std::cos(planeNormalAzimutAngle),
        std::sin(planeNormalPolarAngle) * std::sin(planeNormalAzimutAngle),
        std::cos(planeNormalPolarAngle)
    };
    return planeIntegral(planeNormal, planeDistanceFromOrigin);
}

const float *RadonTransform3D::SingleGPU::resultArray() const
{
    return _resultBuf.hostPtr();
}

void RadonTransform3D::SingleGPU::sliceDimensionsChanged()
{
    _resultBuf = PinnedBufHostRead<float>(_p.nbPatches(), _q);

    const cl_uint2 sliceDim{ _p.dim.width, _p.dim.height };
    _q.enqueueWriteBuffer(_sliceDimBuf, CL_FALSE, 0, 2 * sizeof(uint), &sliceDim);
}

mat::Matrix<3, 4>
RadonTransform3D::SingleGPU::transformXYPlaneToPlane(const mat::Matrix<4, 1>& plane) const
{
    Vector3x1 r1, r2(0.0), r3{ plane.get<0>(), plane.get<1>(), plane.get<2>() };

    // find axis that is as most as perpendicular to r3
    uint axis = std::abs(r3.get<0>()) < std::abs(r3.get<1>()) ? 0 : 1;
    axis = std::abs(r3(axis)) < std::abs(r3.get<2>()) ? axis : 2;
    r2(axis) = 1.0;
    r2 = mat::cross(r3, r2);
    r2 /= r2.norm();
    r1 = mat::cross(r2, r3);

    const auto rotationMatrix = mat::horzcat(mat::horzcat(r1, r2), r3);
    const auto translationVec = rotationMatrix * Vector3x1{ 0.0, 0.0, -plane.get<3>() };

    Matrix3x3 voxelSizeNormalization = mat::diag(
        Vector3x1{ 1.0 / _p.volVoxSize.x, 1.0 / _p.volVoxSize.y, 1.0 / _p.volVoxSize.z });

    return voxelSizeNormalization * mat::horzcat(_p.reso * rotationMatrix, translationVec);
}

} // namespace OCL
} // namespace CTL
