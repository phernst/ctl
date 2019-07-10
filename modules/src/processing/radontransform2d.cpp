#include "radontransform2d.h"
#include "mat/matrix_utils.h"
#include "ocl/clfileloader.h"
#include <QDebug>

const std::string CL_FILE_NAME = "processing/radon2d.cl"; //!< path to .cl file
const std::string CL_KERNEL_NAME = "radon2d"; //!< name of the OpenCL kernel function
const std::string CL_KERNEL_NAME_SUBSET = "radon2dsubset"; //!< name of the OpenCL kernel function
const std::string CL_PROGRAM_NAME = "radonTransform2D"; //!< OCL program name

namespace CTL {
namespace OCL {

/*!
 * Creates a RadonTransform2D instance that allows to compute the 2D Radon transform of \a image.
 * The size of pixels in \a image must be specified by \a pixelSize [in mm]. Data from \a image
 * will be transfered to the OpenCL device \a oclDeviceNb in the device list (as returned by
 * OpenCLConfig::instance().devices()).
 */
RadonTransform2D::RadonTransform2D(const Chunk2D<float> &image, uint oclDeviceNb)
    : _origin({ (image.width() - 1) * 0.5f, (image.height() - 1) * 0.5f })
    , _accuracy(1.0f)
    , _q(OpenCLConfig::instance().context(), OpenCLConfig::instance().devices()[oclDeviceNb])
    , _imgOriginBuf(OpenCLConfig::instance().context(),
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
    OpenCLConfig::instance().addKernel(CL_KERNEL_NAME_SUBSET, clSourceCode, CL_PROGRAM_NAME);

    // Create kernel
    try
    {
        _kernel       = OpenCLConfig::instance().kernel(CL_KERNEL_NAME, CL_PROGRAM_NAME);
        _kernelSubset = OpenCLConfig::instance().kernel(CL_KERNEL_NAME_SUBSET, CL_PROGRAM_NAME);

    } catch(const cl::Error& err)
    {
        qCritical() << "OpenCL error:" << err.what() << "(" << err.err() << ")";
        throw std::runtime_error("OpenCL error");
    }

    if(_kernel == nullptr || _kernelSubset == nullptr)
        throw std::runtime_error("kernel pointer not valid");

    // write buffers
    cl::size_t<3> imgDim;
    imgDim[0] = image.width();
    imgDim[1] = image.height();
    imgDim[2] = 1;

    _q.enqueueWriteBuffer(_imgOriginBuf, CL_FALSE, 0, 2 * sizeof(float), &_origin);
    _q.enqueueWriteImage(_image, CL_FALSE, cl::size_t<3>(), imgDim, 0, 0,
                         const_cast<float*>(image.rawData()));

}

/*!
 * Sets the resolution for line integration to \a stepLength. This defines the step length (in mm)
 * for sampling along the integration lines.
 */
void RadonTransform2D::setAccuracy(float stepLength) { _accuracy = stepLength; }

/*!
 * Sets the origin for the transform to [\a x, \a y] (in pixels).
 */
void RadonTransform2D::setOrigin(float x, float y)
{
    _origin.x = x;
    _origin.y = y;

    _q.enqueueWriteBuffer(_imgOriginBuf, CL_FALSE, 0, 2 * sizeof(float), &_origin);
}

/*!
 * Returns the resolution for line integration. This refers to the step length (in mm) used for
 * sampling along the integration lines.
 */
float RadonTransform2D::accuracy() const { return _accuracy; }

/*!
 * Returns the origin of the transform (in pixels).
 */
mat::Matrix<2, 1> RadonTransform2D::origin() const { return { double(_origin.x), double(_origin.y) }; }

/*!
 * Returns the 2D Radon transform of volume data for the set of sampling points given by \a theta
 * and \a s.
 *
 * This will return the line integral for all combinations of angles \a theta and distances \a s
 * passed.
 *
 * The transform will be computed with respect to the image center, as long as not specified
 * otherwise by calling setOrigin(). The used origin can be checked with origin().
 */
Chunk2D<float> RadonTransform2D::sampleTransform(const std::vector<float>& theta, const std::vector<float>& s) const
{
    const auto& context = OpenCLConfig::instance().context();

    cl::Buffer sVecBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, s.size() * sizeof(float));
    cl::Buffer thetaVecBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, theta.size() * sizeof(float));
    cl::Buffer resultBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, s.size() * theta.size() * sizeof(float));

    // write buffers
    _q.enqueueWriteBuffer(sVecBuf, CL_FALSE, 0, s.size() * sizeof(float), s.data());
    _q.enqueueWriteBuffer(thetaVecBuf, CL_FALSE, 0, theta.size() * sizeof(float), theta.data());

    // set kernel args
    _kernel->setArg(0, _accuracy);
    _kernel->setArg(1, sVecBuf);
    _kernel->setArg(2, thetaVecBuf);
    _kernel->setArg(3, _imgOriginBuf);
    _kernel->setArg(4, resultBuf);
    _kernel->setArg(5, _image);

    // start kernel
    _q.enqueueNDRangeKernel(*_kernel, cl::NullRange, cl::NDRange(s.size(), theta.size()));

    // read result
    Chunk2D<float> ret(uint(theta.size()), uint(s.size()));
    ret.allocateMemory();
    _q.enqueueReadBuffer(resultBuf, CL_TRUE, 0, s.size() * theta.size() * sizeof(float), ret.rawData());

    return ret;
}

std::vector<float> RadonTransform2D::sampleTransform(const std::vector<Radon2DCoord>& smplPts) const
{
    const auto& context = OpenCLConfig::instance().context();

    const auto nbSamples = smplPts.size();

    cl::Buffer smplPtBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, nbSamples * sizeof(cl_float2));
    cl::Buffer resultBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, nbSamples * sizeof(float));

    // write buffers
    _q.enqueueWriteBuffer(smplPtBuf, CL_FALSE, 0, nbSamples * sizeof(cl_float2), smplPts.data());

    // set kernel args
    _kernelSubset->setArg(0, _accuracy);
    _kernelSubset->setArg(1, smplPtBuf);
    _kernelSubset->setArg(2, _imgOriginBuf);
    _kernelSubset->setArg(3, resultBuf);
    _kernelSubset->setArg(4, _image);

    // start kernel
    _q.enqueueNDRangeKernel(*_kernelSubset, cl::NullRange, cl::NDRange(nbSamples));

    // read result
    std::vector<float> ret(nbSamples);
    _q.enqueueReadBuffer(resultBuf, CL_TRUE, 0, nbSamples * sizeof(float), ret.data());

    return ret;
}

/*!
 * Returns the transformation matrix that maps a value on the x-axis to the line specified by
 * \a line (containing the unit normal vector of the line in its first two components and the line's
 * distance from the origin as the third component).
 */
mat::Matrix<2, 3> RadonTransform2D::xAxisToLineMapping(const mat::Matrix<3, 1>& line) const
{
    mat::Matrix<2, 2> Rt{  line.get<1>(), line.get<0>(),
                          -line.get<0>(), line.get<1>() };

    auto t = Rt * mat::Matrix<2, 1>{ 0, -line.get<2>() };

    return mat::horzcat(Rt, t);
}

} // namespace OCL
} // namespace CTL
