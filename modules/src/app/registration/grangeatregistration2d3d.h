#ifndef GRANGEATREGISTRATION2D3D_H
#define GRANGEATREGISTRATION2D3D_H

#include "img/chunk2d.h"
#include "mat/homography.h"
#include "mat/projectionmatrix.h"
#include "processing/errormetrics.h"
#include "processing/volumeresampler.h"
#include <nlopt.hpp>

/*!
 * \class GrangeatRegistration2D3D
 *
 * \brief Grangeat-based 2D/3D registration using NLopt for optimization
 */

class GrangeatRegistration2D3D
{
public:
    CTL::mat::Homography3D optimize(const CTL::Chunk2D<float>& projectionImage,
                                    const CTL::OCL::VolumeResampler& volumeIntermedResampler,
                                    const CTL::mat::ProjectionMatrix& pMat);

    nlopt::opt& optObject();

    const CTL::imgproc::AbstractErrorMetric* metric() const;
    void setMetric(const CTL::imgproc::AbstractErrorMetric* metric);

    float subSamplingLevel() const;
    void setSubSamplingLevel(float subSamplingLevel);

private:
    nlopt::opt _opt{ nlopt::algorithm::LN_SBPLX, 6u };
    const CTL::imgproc::AbstractErrorMetric* _metric = &CTL::metric::L2;
    float _subSamplingLevel = 1.0f;
};

#endif // GRANGEATREGISTRATION2D3D_H
