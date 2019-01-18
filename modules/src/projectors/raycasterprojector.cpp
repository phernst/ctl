#include "raycasterprojector.h"
#include "components/genericdetector.h"
#include "mat/matrix_algorithm.h"
#include "ocl/openclconfig.h"
#include "ocl/clfileloader.h"

#include <QDebug>
#include <exception>

const std::string CL_FILE_NAME_INTERP = "projectors/raycasterprojector_interp.cl"; //!< path to .cl file
const std::string CL_FILE_NAME_NO_INTERP = "projectors/raycasterprojector_no_interp.cl"; //!< path to .cl file
const std::string CL_KERNEL_NAME = "ray_caster"; //!< name of the OpenCL kernel function
const std::string CL_PROGRAM_NAME_INTERP = "rayCaster_interp"; //!< OCL program name for interpolating kernel
const std::string CL_PROGRAM_NAME_NO_INTERP = "rayCaster_noInterp"; //!< OCL program name for non-interpolating kernel

namespace {
struct VolumeSpecs
{
    static VolumeSpecs upSampleVolume(const CTL::VolumeData& volume, uint upSamplingFactor);

    cl::size_t<3> volDim;
    cl_float3 volOffset;
    cl_float3 voxelSize;
    float* volumeDataPtr;
    float mean;
    std::vector<float> upSampledVolume;
};
} // unnamed namespace

namespace CTL {
namespace OCL {

static cl_double16 decomposeM(const Matrix3x3& M);
static cl_float3 determineSource(const ProjectionMatrix& P);
static cl_float3 volumeCorner(cl::size_t<3> volDim, cl_float3 voxelSize, cl_float3 volOffset);

/*!
 * Configures the projector. This extracts all information that is required for projecting with
 * this projector from the \a setup and \a config.
 */
void RayCasterProjector::configure(const AcquisitionSetup &setup,
                                   const AbstractProjectorConfig& config)
{
    // get projection matrices
    _pMats = GeometryEncoder::encodeFullGeometry(setup);

    // extract required system geometry
    auto detectorPixels = setup.system()->detector()->nbPixelPerModule();
    _viewDim.nbRows = detectorPixels.height();
    _viewDim.nbChannels = detectorPixels.width();
    _viewDim.nbModules = setup.system()->detector()->nbDetectorModules();

    // prepare RayCaster
    Q_ASSERT(dynamic_cast<const Config*>(&config));
    _config = static_cast<const Config&>(config);

    Q_ASSERT(_config.raysPerPixel[0]);
    Q_ASSERT(_config.raysPerPixel[1]);
    Q_ASSERT(!qFuzzyIsNull(_config.raySampling));

    if(_config.interpolate) {
        _oclProgramName = CL_PROGRAM_NAME_INTERP;
    }
    else {
        _oclProgramName = CL_PROGRAM_NAME_NO_INTERP;
        _config.volumeUpSampling = 1;
    }

    initOpenCL();
}

/*!
 * Computes the projection of \a volume for all views that have been configured in the configure()
 * step. Returns projection data of all views and detector modules as a ProjectionData object.
 */
ProjectionData RayCasterProjector::project(const VolumeData& volume)
{
    ProjectionData ret(_viewDim);
    if(!volume.hasData())
    {
        qCritical() << "no or contradictory data in volume object";
        return ret;
    }
    if(volume.smallestVoxelSize() <= 0.0f)
        qWarning() << "voxel size is zero or negative";

    const uint nbViews = _pMats.size();
    const size_t pixelPerModule = size_t(_viewDim.nbRows) * _viewDim.nbChannels;
    const size_t pixelPerView = size_t(_viewDim.nbModules) * pixelPerModule;

    // upsample volume (if volumeUpSampling > 1)
    auto volumeSpecs = VolumeSpecs::upSampleVolume(volume, _config.volumeUpSampling);

    // volume specs
    auto& volDim = volumeSpecs.volDim;
    auto& volOffset = volumeSpecs.volOffset;
    auto& voxelSize = volumeSpecs.voxelSize;
    auto* volumeDataPtr = volumeSpecs.volumeDataPtr;

    // determine ray step length in mm
    cl_float smallestVoxelSize = qMin(qMin(voxelSize.s[0], voxelSize.s[1]), voxelSize.s[2]);
    cl_float increment_mm = smallestVoxelSize * _config.raySampling;
    //qDebug() << "ray step length in mm: " << increment_mm;

    try // exception handling
    {
        // check for valid OpenCLConfig
        auto& oclConfig = OpenCLConfig::instance();
        if(!oclConfig.isValid())
            throw std::runtime_error("OpenCLConfig has not been initiated");

        // context and number of used devices
        auto& context = oclConfig.context();
        const auto nbUsedDevs = qMin(uint(_config.deviceIDs.size()), nbViews);
        if(nbUsedDevs == 0)
            throw std::runtime_error("no devices or no views for RayCasterProjector::project");
        qDebug() << "number of used devices for RayCasterProjector: " << nbUsedDevs;

        // Create kernel
        auto* kernel = oclConfig.kernel(CL_KERNEL_NAME, _oclProgramName);
        if(kernel == nullptr)
            throw std::runtime_error("kernel pointer not valid");

        // allocate memory for result
        ret.allocateMemory(nbViews);

        // Create command queues
        std::vector<cl::CommandQueue> queues;
        queues.reserve(nbUsedDevs);
        const auto& availDevices = oclConfig.devices();
        for(uint dev = 0; dev < nbUsedDevs; ++dev)
            queues.emplace_back(context, availDevices[_config.deviceIDs[dev]]);

        // Prepare input data
        cl_uint2 raysPerPixel{ { _config.raysPerPixel[0], _config.raysPerPixel[1] } };
        cl_float3 volCorner = volumeCorner(volDim, voxelSize, volOffset);
        std::vector<cl_float3> sources(nbUsedDevs);
        std::vector<std::vector<cl_double16>> QRs;
        QRs.reserve(nbUsedDevs);
        for(uint dev = 0; dev < nbUsedDevs; ++dev)
            QRs.emplace_back(_viewDim.nbModules);

        // Allocate device buffers and transfer input data to device
        // global constant buffers
        cl_mem_flags readCopyFlag = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;
        cl::Buffer raysPerPixelBuf(context, readCopyFlag, sizeof raysPerPixel, &raysPerPixel);
        cl::Buffer volCornerBuf(context, readCopyFlag, sizeof volCorner, &volCorner);
        cl::Buffer voxelSizeBuf(context, readCopyFlag, sizeof voxelSize, &voxelSize);
        // device specific buffers
        std::vector<cl::Buffer> sourceBufs;
        sourceBufs.reserve(nbUsedDevs);
        for(uint dev = 0; dev < nbUsedDevs; ++dev)
            sourceBufs.emplace_back(context, CL_MEM_READ_ONLY, sizeof(cl_float3));
        std::vector<cl::Buffer> QRBufs;
        QRBufs.reserve(nbUsedDevs);
        for(uint dev = 0; dev < nbUsedDevs; ++dev)
            QRBufs.emplace_back(context, CL_MEM_READ_ONLY, sizeof(cl_double16) * _viewDim.nbModules);

        // Create volume
        std::unique_ptr<cl::Image3D> volumeImg;
        std::unique_ptr<cl::Buffer> volumeBuf;
        std::unique_ptr<cl::Buffer> volumeDimensionsBuf;

        if(_config.interpolate) // volume is cl::Image3D
        {
            volumeImg.reset(new cl::Image3D(context, CL_MEM_READ_ONLY, cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                                            volDim[0], volDim[1], volDim[2]));
            cl::size_t<3> zeroVecOrigin;
            zeroVecOrigin[0] = zeroVecOrigin[1] = zeroVecOrigin[2] = 0;
            for(uint dev = 0; dev < nbUsedDevs; ++dev)
                queues[dev].enqueueWriteImage(*volumeImg, CL_FALSE, zeroVecOrigin, volDim, 0, 0, volumeDataPtr);

        }
        else // no interpolation, volume is cl::Buffer
        {
            volumeBuf.reset(new cl::Buffer(context, CL_MEM_READ_ONLY,
                                           sizeof(float) * volDim[0] * volDim[1] * volDim[2]));
            for(uint dev = 0; dev < nbUsedDevs; ++dev)
                queues[dev].enqueueWriteBuffer(*volumeBuf, CL_FALSE, 0,
                                               sizeof(float) * volDim[0] * volDim[1] * volDim[2],
                                               volumeDataPtr);
            uint volumeDim[3] = { uint(volDim[0]), uint(volDim[1]), uint(volDim[2]) };
            volumeDimensionsBuf.reset(new cl::Buffer(context, readCopyFlag, sizeof volumeDim, volumeDim));
        }

        // Allocate output buffer: projection buffer for each device
        std::vector<cl::Buffer> projectionBufs;
        projectionBufs.reserve(nbUsedDevs);
        for(uint dev = 0; dev < nbUsedDevs; ++dev)
            projectionBufs.emplace_back(context, CL_MEM_WRITE_ONLY, sizeof(float) * pixelPerView);

        // Set kernel parameters for each device
        std::vector<cl::Kernel> kernels;
        kernels.reserve(nbUsedDevs);
        for(uint dev = 0; dev < nbUsedDevs; ++dev)
            kernels.push_back(*kernel);
        for(uint dev = 0; dev < nbUsedDevs; ++dev)
        {
            kernels[dev].setArg(0, increment_mm);
            kernels[dev].setArg(1, raysPerPixelBuf);
            kernels[dev].setArg(2, sourceBufs[dev]);
            kernels[dev].setArg(3, volCornerBuf);
            kernels[dev].setArg(4, voxelSizeBuf);
            kernels[dev].setArg(5, QRBufs[dev]);
            kernels[dev].setArg(6, projectionBufs[dev]);

            if(_config.interpolate)
            {
                kernels[dev].setArg(7, *volumeImg);
            }
            else
            {
                kernels[dev].setArg(7, *volumeBuf);
                kernels[dev].setArg(8, *volumeDimensionsBuf);
            }
        }

        std::vector<cl::Event> lastModule(nbUsedDevs);
        SingleViewData* currentViewData;
        const SingleViewGeometry* currentViewPMats;

        // loop over all projections
        uint dev = 0;
        for(uint view = 0; view < nbViews; ++view)
        {
            currentViewPMats = &_pMats.at(view);
            currentViewData = &ret.view(view);

            // all modules have same source position --> use first module PMat (arbitrary)
            sources[dev] = determineSource(currentViewPMats->first());
            queues[dev].enqueueWriteBuffer(sourceBufs[dev], CL_FALSE, 0, sizeof(cl_float3),
                                           &sources[dev]);

            // individual module geometry: QR is only determined by M, where P=[M|p4]
            for(uint module = 0; module < _viewDim.nbModules; ++module)
                QRs[dev][module] = decomposeM(currentViewPMats->at(module).M());

            queues[dev].enqueueWriteBuffer(QRBufs[dev], CL_FALSE, 0,
                                           sizeof(cl_double16) * _viewDim.nbModules,
                                           QRs[dev].data());

            // Launch kernel on the compute device.
            queues[dev].enqueueNDRangeKernel(kernels[dev],
                                             cl::NullRange,
                                             cl::NDRange(_viewDim.nbChannels,
                                                         _viewDim.nbRows,
                                                         _viewDim.nbModules));

            // Get result back to host.
            for(uint module = 0; module < _viewDim.nbModules; ++module)
                queues[dev].enqueueReadBuffer(projectionBufs[dev], CL_FALSE,
                                              sizeof(float) * pixelPerModule * module,
                                              sizeof(float) * pixelPerModule,
                                              currentViewData->module(module).rawData(), nullptr,
                                              &lastModule[dev]);

            // increment to next device
            dev = ++dev % nbUsedDevs;
            if(dev == 0)
                for(uint dev = 0; dev < nbUsedDevs; ++dev)
                    lastModule[dev].wait();

            emit notifier()->projectionFinished(view);
        }
        // wait for remaining devices
        for(uint remainingDev = 0; remainingDev < dev; ++remainingDev)
            lastModule[remainingDev].wait();

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error: " << err.what() << "(" << err.err() << ")";
    } catch(const std::bad_alloc& except)
    {
        qCritical() << "Allocation error: " << except.what();
    } catch(const std::exception& except)
    {
        qCritical() << "std exception: " << except.what();
    }

    return ret;
}

/*!
 * Initializes the OpenCL environment (using OpenCLConfig).
 *
 * This also includes loading the .cl kernel file that contain the OpenCL kernel source code.
 * It does not change the OpenCLConfig device settings or it uses the default settings of
 * OpenCLConfig.
 * Throws std::runtime_error if
 * \li the .cl kernel file is not readable (e.g. file does not exists)
 * \li OpenCLConfig is not valid
 * \li `deviceID` of the RayCasterProjector::Config exceeds the size of the OpenCLConfig device list
 * \li an OpenCL exception is thrown.
 */
void RayCasterProjector::initOpenCL()
{
    try // OCL exception catching
    {
        auto& oclConfig = OpenCLConfig::instance();
        // general checks
        if(!oclConfig.isValid())
            throw std::runtime_error("OpenCLConfig is not valid");
        // check for invalid device IDs
        for(auto devID : _config.deviceIDs)
            if(devID >= oclConfig.devices().size())
                throw std::runtime_error("device ID is not available.\nID = " +
                                         std::to_string(devID) +
                                         "\nnumber of devices = " +
                                         std::to_string(oclConfig.devices().size()));
        // if not deviceIDs specified, use all available devices
        if(_config.deviceIDs.empty())
        {
            _config.deviceIDs.resize(oclConfig.devices().size());
            std::iota(_config.deviceIDs.begin(), _config.deviceIDs.end(), 0);
        }

        // check if required kernel is provided
        if(oclConfig.kernelExists(CL_KERNEL_NAME, _oclProgramName))
            return;

        // load source code from file
        const auto& clFileName = _config.interpolate
                                ? CL_FILE_NAME_INTERP
                                : CL_FILE_NAME_NO_INTERP;
        ClFileLoader clFile(clFileName);
        if(!clFile.isValid())
            throw std::runtime_error(clFileName + "\nis not readable");
        const auto clSourceCode = clFile.loadSourceCode();

        // add kernel to OCLConfig
        if(!oclConfig.addKernel(CL_KERNEL_NAME, clSourceCode, _oclProgramName))
            qDebug() << "no kernel added: kernel name "
                     << QString::fromStdString(CL_KERNEL_NAME) << " within ocl program "
                     << QString::fromStdString(_oclProgramName) << " already exists";

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error: " << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }
}

// documentation in "\file section" at end of this file
cl_double16 decomposeM(const Matrix3x3& M)
{
    auto QR = mat::QRdecomposition(M);
    auto& Q = QR.Q;
    auto& R = QR.R;
    if(std::signbit(R(0, 0) * R(1, 1) * R(2, 2)))
        R = -R;
    cl_double16 ret = { {Q(0,0),Q(0,1),Q(0,2),
                         Q(1,0),Q(1,1),Q(1,2),
                         Q(2,0),Q(2,1),Q(2,2),
                         R(0,0),R(0,1),R(0,2),
                                R(1,1),R(1,2),
                                       R(2,2)} };
    return ret;
}

// documentation in "\file section" at end of this file
cl_float3 determineSource(const ProjectionMatrix& P)
{
    auto ret = P.sourcePosition();
    return { { static_cast<float>(ret.get<0>()), static_cast<float>(ret.get<1>()),
               static_cast<float>(ret.get<2>()) } };
}

// documentation in "\file section" at end of this file
cl_float3 volumeCorner(cl::size_t<3> volDim, cl_float3 voxelSize, cl_float3 volOffset)
{
    return { { volOffset.s[0] - 0.5f * volDim[0] * voxelSize.s[0],
               volOffset.s[1] - 0.5f * volDim[1] * voxelSize.s[1],
               volOffset.s[2] - 0.5f * volDim[2] * voxelSize.s[2] } };
}

RayCasterProjector::Config RayCasterProjector::Config::optimizedFor(const VolumeData &volume,
                                                                    const AbstractDetector &detector)
{
    Config ret;

    // opimlized number of rays
    double Xmm = detector.pixelDimensions().width();
    double Ymm = detector.pixelDimensions().height();
    if(Xmm==0.0 || Ymm==0.0)
        throw std::runtime_error("pixel dimensions are singular");
    double pixelRatio = Xmm / Ymm;
    bool broadPixel = pixelRatio > 1.0;
    if(broadPixel)
        pixelRatio = 1.0 / pixelRatio;
    // test a preset of ratios (max 4 rays per dimension)
    uint ratios[5][2] = { {1,2}, {1,3}, {1,4}, {2,3}, {3,4} };
    double deviation = fabs(pixelRatio - 1.0);
    for(const auto& r : ratios)
        if(fabs(pixelRatio - (double)r[0]/r[1]) < deviation)
        {
            deviation = fabs(pixelRatio - (double)r[0]/r[1]);
            ret.raysPerPixel[0] = r[broadPixel];
            ret.raysPerPixel[1] = r[!broadPixel];
        }

    // up-sampling factor for volume (if voxel large)
    double smallestVoxelSize = volume.smallestVoxelSize();
    double smallestPixelSize = qMin(Xmm/ret.raysPerPixel[0], Ymm/ret.raysPerPixel[1]);
    ret.volumeUpSampling = qMax(uint(smallestVoxelSize / smallestPixelSize), 1u);

    // increase number of rays (if voxels are small)
    uint rayIncreaseFactor = qMax(uint(smallestPixelSize / smallestVoxelSize), 1u);
    ret.raysPerPixel[0] *= rayIncreaseFactor;
    ret.raysPerPixel[1] *= rayIncreaseFactor;

    //qDebug() << "raysPerPixel:" << ret.raysPerPixel[0] << "," << ret.raysPerPixel[1];
    //qDebug() << "upsampling factor:" << ret.volumeUpSampling;

    return ret;
}

} // namespace OCL
} // namespace CTL

VolumeSpecs VolumeSpecs::upSampleVolume(const CTL::VolumeData& volume, uint upSamplingFactor)
{
    VolumeSpecs volSpecs;

    // offset is same for all cases
    volSpecs.volOffset.s[0] = volume.offset().x;
    volSpecs.volOffset.s[1] = volume.offset().y;
    volSpecs.volOffset.s[2] = volume.offset().z;

    size_t X = volume.nbVoxels().x,
           Y = volume.nbVoxels().y,
           Z = volume.nbVoxels().z;

    switch (upSamplingFactor) {

    case 0: // 1 Voxel

        volSpecs.volDim[0] = 1;
        volSpecs.volDim[1] = 1;
        volSpecs.volDim[2] = 1;

        volSpecs.voxelSize.s[0] = volume.voxelSize().x * X;
        volSpecs.voxelSize.s[1] = volume.voxelSize().y * Y;
        volSpecs.voxelSize.s[2] = volume.voxelSize().z * Z;

        // mean value
        volSpecs.mean = 0.0f;
        for(auto val : volume.constData())
            volSpecs.mean += val;
        volSpecs.mean /= (float)volume.totalVoxelCount();

        volSpecs.volumeDataPtr = &volSpecs.mean;

        break;
    case 1: // no changes

        volSpecs.volDim[0] = X;
        volSpecs.volDim[1] = Y;
        volSpecs.volDim[2] = Z;

        volSpecs.voxelSize.s[0] = volume.voxelSize().x;
        volSpecs.voxelSize.s[1] = volume.voxelSize().y;
        volSpecs.voxelSize.s[2] = volume.voxelSize().z;

        // const cast due to poor windows OpenCL API
        volSpecs.volumeDataPtr = const_cast<float*>(volume.rawData());

        break;
    default: // upsampling

        size_t newX = X * upSamplingFactor;
        size_t newY = Y * upSamplingFactor;
        size_t newZ = Z * upSamplingFactor;
        volSpecs.volDim[0] = newX;
        volSpecs.volDim[1] = newY;
        volSpecs.volDim[2] = newZ;

        volSpecs.voxelSize.s[0] = volume.voxelSize().x / float(upSamplingFactor);
        volSpecs.voxelSize.s[1] = volume.voxelSize().y / float(upSamplingFactor);
        volSpecs.voxelSize.s[2] = volume.voxelSize().z / float(upSamplingFactor);

        // allocate memory
        volSpecs.upSampledVolume = std::vector<float>(newX * newY * newZ);

        for(uint z = 0; z < Z; ++z)
            for(uint y = 0; y < Y; ++y)
                for(uint x = 0; x < X; ++x)
                {
                    float val = volume(x,y,z);
                    for(size_t innerZ = 0; innerZ < upSamplingFactor; ++innerZ)
                        for(size_t innerY = 0; innerY < upSamplingFactor; ++innerY)
                            for(size_t innerX = 0; innerX < upSamplingFactor; ++innerX)
                            {
                                size_t lookUp = x * upSamplingFactor + innerX
                                             + (y * upSamplingFactor + innerY) * newX
                                             + (z * upSamplingFactor + innerZ) * newX * newY;
                                volSpecs.upSampledVolume[lookUp] = val;
                            }
                }

        volSpecs.volumeDataPtr = volSpecs.upSampledVolume.data();

        break;
    }

//    qDebug() << "project | voxel size: "
//             << volSpecs.voxelSize.s[0]
//             << volSpecs.voxelSize.s[1]
//             << volSpecs.voxelSize.s[2];
//    qDebug() << "project | voxel dim:  "
//             << volSpecs.volDim[0]
//             << volSpecs.volDim[1]
//             << volSpecs.volDim[2];

    return volSpecs;
}

/*! \file */
///@{
/*!
 * \fn static cl_double16 CTL::OCL::decomposeM(const Matrix3x3& M)
 * \relates CTL::OCL::RayCasterProjector
 *
 * Performs a QR-decomposition of \a M and returns the results as a `cl_double16`.\n
 * \f$Q\f$: orthogonal matrix (with \f$\mathrm{det}\,Q=1\f$),\n
 * \f$R\f$: upper triangular matrix.\n
 * Moreover, \f$R\f$ is multiplied with the sign of its determinant with the result that
 * \f$\mathrm{det}\,R\geq 0\f$.
 *
 * The QR representation of \a M is used inside the OpenCL kernel to calculate the ray direction
 * \f$\mathbf{d}\f$ to a detector pixel position \f$[x,y]\f$ (channel, row) by using the relation
 * \f$\mathbf{d}=M^{-1} [x,y,1]^T=R^{-1} Q^T [x,y,1]^T\f$.
 * Since a positive sign of \f$\mathrm{det}\,R=\mathrm{det}\,M\f$ is ensured, the orientation of the
 * direction vector \f$\mathbf{d}\f$ is in a way that it points always from source to detector.
 *
 * In the returned value, \f$Q\f$ and \f$R\f$ are stored such that the first nine elements are used
 * for \f$Q\f$ (row major order), followed by the upper triangle
 * \f$[R_{11},R_{12},R_{13},R_{22},R_{23},R_{33}]\f$ of \f$R\f$ in the following six elements.
 * The last element of the returned vector of type `cl_double16` is zero and has no meaning.
 *
 * \sa CTL::mat::QRdecomposition().
 */

/*!
 * \fn static cl_float3 CTL::OCL::determineSource(const ProjectionMatrix& P);
 * \relates CTL::OCL::RayCasterProjector
 *
 * Returns the source position encoded in \a P as a `cl_float3`.
 */

/*!
 * \fn static cl_float3 CTL::OCL::volumeCorner(cl::size_t<3> volDim, cl_float3 voxelSize, cl_float3 volOffset);
 * \relates CTL::OCL::RayCasterProjector
 *
 * Computes and returns the coordinates of the volume corner (with the lowest coordinates) with
 * dimensions \a volDim and voxel size \a voxelSize under consideration of an offset \a volOffset
 * as a `cl_float3`.
 */
///@}
