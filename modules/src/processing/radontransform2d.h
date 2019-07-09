#ifndef RADONTRANSFORM2D_H
#define RADONTRANSFORM2D_H

#include <img/chunk2d.h>
#include <mat/matrix.h>
#include "ocl/openclconfig.h"

namespace CTL {
namespace OCL {

/*!
 * \class RadonTransform2D
 * \brief Allows to compute the 2D Radon transform of Chunk2D<float> data.
 *
 * This class allows to compute the 2D Radon transform of Chunk2D<float> data. The 2D Radon
 * transform describes an image by all its line integrals.
 * Computation is carried out on an OpenCL device, making use of fast hardware interpolation on
 * texture memory.
 *
 * To compute the 2D Radon transform of image data, create a RadonTransform2D instance by passing
 * to its constructor a (constant) reference to the Chunk2D with the image data that you want to
 * compute the transform of. The image data will be transfered to the OpenCL device immediately
 * (stored internally as cl::Image2D).
 *
 * The Radon transform of the image can be computed for a given set of sampling points (i.e. angles
 * and distances) using sampleTransform(). This will return the line integral for all combinations
 * of angles and distances passed to sampleTransform(). Computation is parallelized over all sample
 * points on the OpenCL device.
 *
 * By default, the origin for the transform (i.e. the point which all lines with distance zero go
 * through) and the resolution (ie. step length) within the integration line are determined
 * automatically - w.r.t. the image specs - using the following rules:
 *
 * Origin [x, y]: [(nbPixels.x - 1) * 0.5, (nbPixels.y - 1) * 0.5)]
 * Resolution (step length): min(pixelSize.x, pixelSize.y)
 *
 * If necessary, both settings can be changed using setOrigin() and setLineResolution(),
 * respectively.
 */
class RadonTransform2D
{
public:
    RadonTransform2D(const Chunk2D<float>& image, const mat::Matrix<2, 1>& pixelSize, uint oclDeviceNb = 0);

    void setLineResolution(float stepLength);
    void setOrigin(float x, float y);

    float lineResolution() const;
    mat::Matrix<2, 1> origin() const;

    Chunk2D<float> sampleTransform(const std::vector<float>& theta,
                                   const std::vector<float>& s) const;

    std::vector<float> sampleTransform(const std::vector<Radon2DCoord>& smplPts) const;

    mat::Matrix<2, 3> xAxisToLineMapping(const mat::Matrix<3, 1>& line) const;

private:
    Chunk2D<float>::Dimensions _imageDim; //!< image dimensions
    cl_float2 _pixelSize; //!< pixel size (mm)
    cl_float2 _origin; //!< pixel coordinate of Radon transform's origin
    float _lineReso; //!< discretization of the line integral

    //ocl
    cl::CommandQueue _q; //!< OpenCL command queue
    cl::Kernel* _kernel; //!< Pointer to OpenCL kernel
    cl::Kernel* _kernelSubset; //!< Pointer to OpenCL kernel (for subset of transform)
    cl::Buffer _imgResoBuf; //!< Buffer for image resolution
    cl::Buffer _imgOriginBuf; //!< Buffer for image origing
    cl::Image2D _image; //!< Image data to be transformed

};

} // namespace OCL
} // namespace CTL

#endif // RADONTRANSFORM2D_H
