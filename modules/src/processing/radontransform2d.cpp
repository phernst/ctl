#include "radontransform2d.h"
#include "mat/matrix_utils.h"
#include "ocl/clfileloader.h"
#include <QDebug>

const std::string CL_FILE_NAME = "processing/radon2d.cl"; //!< path to .cl file
const std::string CL_KERNEL_NAME = "radon2d"; //!< name of the OpenCL kernel function
const std::string CL_PROGRAM_NAME = "radonTransform2D"; //!< OCL program name

namespace CTL {
namespace OCL {

RadonTransform2D::RadonTransform2D(const Chunk2D<float> &image, const mat::Matrix<2, 1>& pixelSize, uint oclDeviceNb)
    : _imageDim(image.dimensions())
    , _pixelSize({ float(pixelSize.get<0>()), float(pixelSize.get<1>()) })
    , _origin((image.width() - 1) * 0.5, (image.height() - 1) * 0.5)
    , _lineReso(std::min(pixelSize.get<0>(), pixelSize.get<1>()))
    , _q(OpenCLConfig::instance().context(), OpenCLConfig::instance().devices()[oclDeviceNb])
    , _imgResoBuf(OpenCLConfig::instance().context(),
                  CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                  2 * sizeof(float))
    , _image(OpenCLConfig::instance().context(),
             CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
             cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
             image.width(),
             image.height())
{
    OCL::ClFileLoader clFile(CL_FILE_NAME);
    if(!clFile.isValid())
        throw std::runtime_error(CL_FILE_NAME + "\nis not readable");
    const auto clSourceCode = clFile.loadSourceCode();

    OpenCLConfig::instance().addKernel(CL_KERNEL_NAME, clSourceCode, CL_PROGRAM_NAME);

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
    cl::size_t<3> imgDim;
    imgDim[0] = image.width();
    imgDim[1] = image.height();
    imgDim[2] = 1;

    _q.enqueueWriteBuffer(_imgResoBuf, CL_FALSE, 0, 2 * sizeof(float), &_pixelSize);
    _q.enqueueWriteImage(_image, CL_FALSE, cl::size_t<3>(), imgDim, 0, 0,
                         const_cast<float*>(image.rawData()));

}

void RadonTransform2D::setLineResolution(float stepLength) { _lineReso = stepLength; }

void RadonTransform2D::setOrigin(double x, double y)
{
    _origin.get<0>() = x;
    _origin.get<1>() = y;
}

float RadonTransform2D::lineResolution() const { return _lineReso; }

const mat::Matrix<2, 1>& RadonTransform2D::origin() const { return _origin; }

Chunk2D<float> RadonTransform2D::sampleTransform(const std::vector<float>& theta, const std::vector<float>& s) const
{
/*
 *__kernel void radon2d( float lineReso,
                       __constant float* sVec,
                       __constant float* thetaVec,
                       __constant float2* imgReso,
                       __global float* result,
                       __read_only image2d_t image )
                       */
    const auto& context = OpenCLConfig::instance().context();

    cl::Buffer sVecBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, s.size() * sizeof(float));
    cl::Buffer thetaVecBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, theta.size() * sizeof(float));
    cl::Buffer resultBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, s.size() * theta.size() * sizeof(float));

    // write buffers
    _q.enqueueWriteBuffer(sVecBuf, CL_FALSE, 0, s.size() * sizeof(float), s.data());
    _q.enqueueWriteBuffer(thetaVecBuf, CL_FALSE, 0, theta.size() * sizeof(float), theta.data());

    // set kernel args
    _kernel->setArg(0, _lineReso);
    _kernel->setArg(1, sVecBuf);
    _kernel->setArg(2, thetaVecBuf);
    _kernel->setArg(3, _imgResoBuf);
    _kernel->setArg(4, resultBuf);
    _kernel->setArg(5, _image);

    // start kernel
    _q.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(s.size(), theta.size()));

    // read result
    Chunk2D<float> ret(theta.size(), s.size());
    ret.allocateMemory();
    _q.enqueueReadBuffer(resultBuf, CL_TRUE, 0, s.size() * theta.size() * sizeof(float), ret.rawData());

    return ret;
}

mat::Matrix<2, 3> RadonTransform2D::xAxisToLineMapping(const mat::Matrix<3, 1>& plane) const
{
    mat::Matrix<2, 2> Rt{ plane.get<1>(),  plane.get<0>(),
                          -plane.get<0>(), plane.get<1>() };

    auto t = Rt * mat::Matrix<2, 1>{ 0, -plane.get<2>() };

    return mat::horzcat(Rt, t);
}

} // namespace OCL
} // namespace CTL
