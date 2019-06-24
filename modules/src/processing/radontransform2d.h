#ifndef RADONTRANSFORM2D_H
#define RADONTRANSFORM2D_H

#include <img/chunk2d.h>
#include <mat/matrix.h>
#include "ocl/openclconfig.h"

namespace CTL {
namespace OCL {

class RadonTransform2D
{
public:
    RadonTransform2D(const Chunk2D<float>& image, const mat::Matrix<2, 1>& pixelSize, uint oclDeviceNb = 0);

    void setLineResolution(float stepLength);
    void setOrigin(double x, double y);

    float lineResolution() const;
    const mat::Matrix<2, 1>& origin() const;

    Chunk2D<float> sampleTransform(const std::vector<float>& theta,
                                   const std::vector<float>& s) const;

    mat::Matrix<2, 3> xAxisToLineMapping(const mat::Matrix<3, 1>& plane) const;

private:
    Chunk2D<float>::Dimensions _imageDim; //!< image dimensions
    cl_float2 _pixelSize; //!< pixel size (mm)
    mat::Matrix<2, 1> _origin; //!< pixel coordinate of Radon transform's origin
    float _lineReso; //!< discretization of the line integral

    //ocl
    cl::CommandQueue _q;
    cl::Kernel* _kernel;
    cl::Buffer _imgResoBuf;
    cl::Image2D _image;
};

} // namespace OCL
} // namespace CTL

#endif // RADONTRANSFORM2D_H
