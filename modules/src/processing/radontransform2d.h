#ifndef RADONTRANSFORM2D_H
#define RADONTRANSFORM2D_H

#include <img/chunk2d.h>
#include <mat/matrix.h>
#include "ocl/openclconfig.h"
#include "processing/coordinates.h"

namespace CTL {

struct Radon2DCoord : Generic2DCoord
{
    Radon2DCoord() = default;
    Radon2DCoord(float angle, float distance) : Generic2DCoord (angle, distance) {}

    float& angle() { return data[0]; }
    float& dist() { return data[1]; }
    const float& angle() const { return data[0]; }
    const float& dist() const { return data[1]; }
};

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
    explicit RadonTransform2D(const Chunk2D<float>& image, uint oclDeviceNb = 0);

    void setAccuracy(float stepLength);
    void setOrigin(float x, float y);

    float accuracy() const;
    mat::Matrix<2, 1> origin() const;

    Chunk2D<float> sampleTransform(const std::vector<float>& theta,
                                   const std::vector<float>& s) const;

    std::vector<float> sampleTransform(const std::vector<Radon2DCoord>& smplPts) const;

    mat::Matrix<2, 3> xAxisToLineMapping(const mat::Matrix<3, 1>& line) const;

private:
    cl_float2 _origin; //!< pixel coordinate of Radon transform's origin
    float _accuracy; //!< discretization of the line integral as a fraction of the minimum pixel size

    //ocl
    cl::CommandQueue _q; //!< OpenCL command queue
    cl::Kernel* _kernel; //!< Pointer to OpenCL kernel
    cl::Kernel* _kernelSubset; //!< Pointer to OpenCL kernel (for subset of transform)
    cl::Buffer _imgOriginBuf; //!< Buffer for image origing
    cl::Image2D _image; //!< Image data to be transformed
};

std::vector<Generic2DCoord> toGeneric2DCoord(const std::vector<Radon2DCoord>& radonCoord);

} // namespace OCL
} // namespace CTL

#endif // RADONTRANSFORM2D_H
