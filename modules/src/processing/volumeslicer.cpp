#include "volumeslicer.h"
#include "mat/matrix_utils.h"
#include "ocl/clfileloader.h"

#include <QDebug>

namespace CTL {
namespace OCL {

const std::string CL_FILE_NAME = "processing/volumeSlicer.cl"; //!< path to .cl file
const std::string CL_KERNEL_NAME = "slicer"; //!< name of the OpenCL kernel function
const std::string CL_PROGRAM_NAME = "volumeSlicer"; //!< OCL program name

VolumeSlicer::VolumeSlicer(const VoxelVolume<float>& volume, uint oclDeviceNb)
    : _dim(sliceDim(volume.dimensions()))
    , _reso(volume.smallestVoxelSize())
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
    , _sliceBuf(_dim.height * _dim.width, _q)
    , _volDim(volume.dimensions())
    , _volOffset(volume.offset())
    , _volVoxSize(volume.voxelSize())
{
    cl::size_t<3> volDim;
    volDim[0] = volume.dimensions().x;
    volDim[1] = volume.dimensions().y;
    volDim[2] = volume.dimensions().z;
    _q.enqueueWriteImage(_volImage3D, CL_TRUE, cl::size_t<3>(), volDim, 0, 0,
                         const_cast<float*>(volume.rawData()));

    // check for valid OpenCLConfig
    auto& oclConfig = OpenCLConfig::instance();
    if(!oclConfig.isValid())
        throw std::runtime_error("OpenCLConfig has not been initiated");

    OCL::ClFileLoader clFile(CL_FILE_NAME);
    if(!clFile.isValid())
        throw std::runtime_error(CL_FILE_NAME + "\nis not readable");
    const auto clSourceCode = clFile.loadSourceCode();

    oclConfig.addKernel(CL_KERNEL_NAME, clSourceCode, CL_PROGRAM_NAME);

    // Create kernel
    try
    {
        _kernel = oclConfig.kernel(CL_KERNEL_NAME, CL_PROGRAM_NAME);

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    if(_kernel == nullptr)
        throw std::runtime_error("kernel pointer not valid");
}

void VolumeSlicer::setSliceDimensions(Chunk2D<float>::Dimensions dimensions) { _dim = dimensions; }

void VolumeSlicer::setSliceResolution(float pixelResolution) { _reso = pixelResolution; }

Chunk2D<float>::Dimensions VolumeSlicer::sliceDimensions() const { return _dim; }

float VolumeSlicer::sliceResolution() const { return _reso; }

Chunk2D<float> VolumeSlicer::slice(const mat::Matrix<3, 1>& planeUnitNormal,
                                   double planeDistanceFromOrigin) const
{
    Q_ASSERT(qFuzzyCompare(planeUnitNormal.norm(), 1.0));

    Chunk2D<float> ret(_dim);

    try {
        if(_volVoxSize.x <= 0.0f || _volVoxSize.y <= 0.0f || _volVoxSize.z <= 0.0f)
            throw std::runtime_error("voxel size is zero or negative");

        // calculate homography that maps a XY-plane to the requested plane
        const auto h = createInverseTransformationToXYPlane(
            mat::vertcat(planeUnitNormal, mat::Matrix<1, 1>(-planeDistanceFromOrigin)));
        // store in OpenCL specifiv vector format
        const cl_float16 h_cl{ float(h(0, 0)), float(h(0, 1)), float(h(0, 2)), float(h(0, 3)),
                               float(h(1, 0)), float(h(1, 1)), float(h(1, 2)), float(h(1, 3)),
                               float(h(2, 0)), float(h(2, 1)), float(h(2, 2)), float(h(2, 3)) };

        const cl_uint2 sliceDim{ _dim.width, _dim.height };

        const cl_float3 voxCorner{ -0.5f * (_volDim.x - 1) + _volOffset.x / _volVoxSize.x,
                                   -0.5f * (_volDim.y - 1) + _volOffset.y / _volVoxSize.y,
                                   -0.5f * (_volDim.z - 1) + _volOffset.z / _volVoxSize.z };
        // write buffers
        _q.enqueueWriteBuffer(_homoBuf, CL_FALSE, 0, 12 * sizeof(float), &h_cl);
        _q.enqueueWriteBuffer(_sliceDimBuf, CL_FALSE, 0, 2 * sizeof(uint), &sliceDim);
        _q.enqueueWriteBuffer(_voxCornerBuf, CL_FALSE, 0, 3 * sizeof(float), &voxCorner);

        // set kernel arguments and run
        _kernel->setArg(0, _voxCornerBuf);
        _kernel->setArg(1, _sliceDimBuf);
        _kernel->setArg(2, _homoBuf);
        _kernel->setArg(3, _sliceBuf.devBuffer());
        _kernel->setArg(4, _volImage3D);

        _q.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(_dim.width, _dim.height));

        // read result
        ret.allocateMemory();
        _sliceBuf.readFromDev(ret.rawData());

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
    
    return ret;
}

Chunk2D<float> VolumeSlicer::slice(double planeNormalAzimutAngle,
                                   double planeNormalPolarAngle,
                                   double planeDistanceFromOrigin) const
{
    mat::Matrix<3, 1> planeNormal{
        std::sin(planeNormalPolarAngle) * std::cos(planeNormalAzimutAngle),
        std::sin(planeNormalPolarAngle) * std::sin(planeNormalAzimutAngle),
        std::cos(planeNormalPolarAngle)
    };
    return slice(planeNormal, planeDistanceFromOrigin);
}

const VoxelVolume<float>::Dimensions& VolumeSlicer::volDim() const
{
    return _volDim;
}

const VoxelVolume<float>::Offset& VolumeSlicer::volOffset() const
{
    return _volOffset;
}

const VoxelVolume<float>::VoxelSize& VolumeSlicer::volVoxSize() const
{
    return _volVoxSize;
}

mat::Matrix<3, 4>
VolumeSlicer::createInverseTransformationToXYPlane(const mat::Matrix<4, 1>& plane) const
{
    mat::Matrix<3, 1> r1, r2(0.0), r3{ plane.get<0>(), plane.get<1>(), plane.get<2>() };

    // find axis that is as most as perpendicular to r3
    uint axis = std::abs(r3.get<0>()) < std::abs(r3.get<1>()) ? 0 : 1;
    axis = std::abs(r3(axis)) < std::abs(r3.get<2>()) ? axis : 2;
    r2(axis) = 1.0;
    r2 = mat::cross(r3, r2);
    r2 /= r2.norm();
    r1 = mat::cross(r2, r3);

    const auto rotationMatrix = mat::horzcat(mat::horzcat(r1, r2), r3);
    const auto translationVec = rotationMatrix * mat::Matrix<3, 1>{ 0.0, 0.0, -plane.get<3>() };

    return mat::diag(Vector3x1{ 1.0 / _volVoxSize.x, 1.0 / _volVoxSize.y, 1.0 / _volVoxSize.z })
        * mat::horzcat(_reso * rotationMatrix, translationVec);
}

Chunk2D<float>::Dimensions VolumeSlicer::sliceDim(const VoxelVolume<float>::Dimensions& volDim)
{
    auto sliceDim = std::max(std::max(volDim.x, volDim.y), volDim.z);
    sliceDim = std::ceil(1.4142f * sliceDim); // sqrt(2) times larger
    return { sliceDim, sliceDim };
}

} // namespace OCL
} // namespace CTL
