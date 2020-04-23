#include "grangeatregistration2d3d.h"
#include "processing/consistency.h"
#include <QDebug>

namespace CTL {
namespace NLOPT {

namespace {

struct DataForOptimization
{
    const std::shared_ptr<const std::vector<float>>& projIntermedFct;
    const OCL::VolumeResampler& volumeIntermedResampler;
    const OCL::Radon3DCoordTransform& radon3DCoordTransform;
    const imgproc::AbstractErrorMetric& metric;
};

float computeDeltaS(const OCL::VolumeResampler& volIntermedFct, const mat::ProjectionMatrix& pMat);
double objFct(const std::vector<double>& x, std::vector<double>& grad, void* data);
Matrix3x3 rotationMatrix(const Vector3x1& axis);
void generateMsg(nlopt::result errCode);

} // unnamed namespace

mat::Homography3D GrangeatRegistration2D3D::optimize(const Chunk2D<float>& projectionImage,
                                                     const OCL::VolumeResampler& volumeIntermedResampler,
                                                     const mat::ProjectionMatrix& pMat)
{
    // calculate s spacing in sinogram
    const auto deltaS = computeDeltaS(volumeIntermedResampler, pMat);
    Q_ASSERT(deltaS > 0.0f);

    // initialize IntermedGen2D3D
    OCL::IntermedGen2D3D gen;
    gen.setLineDistance(deltaS);
    if(_subSamplingLevel != 1.0f)
        gen.setSubsampleLevel(_subSamplingLevel);

    // calculate initial intermediate functions
    auto initalIntermedFctPair = gen.intermedFctPair(projectionImage, pMat, volumeIntermedResampler);
    qDebug() << "initial inconsistency:" << initalIntermedFctPair.inconsistency(*_metric);

    // initialize transformation of 3d Radon coordinates
    const OCL::Radon3DCoordTransform transf(gen.lastSampling());

    // initialize optimization
    DataForOptimization data{ initalIntermedFctPair.ptrToFirst(), volumeIntermedResampler, transf,
                              *_metric };
    _opt.set_min_objective(objFct, &data);

    // perform optimization
    std::vector<double> param(6, 0.0);
    double remainInconsistency;
    auto errCode = _opt.optimize(param, remainInconsistency);

    // output messages
    qDebug() << "remaining inconsistency:" << remainInconsistency;
    generateMsg(errCode);

    return Homography3D(rotationMatrix({ param[0], param[1], param[2] }), // rotation matrix
                        Vector3x1{ param[3], param[4], param[5] });       // translation vector
}

nlopt::opt& GrangeatRegistration2D3D::optObject() { return _opt; }

const imgproc::AbstractErrorMetric* GrangeatRegistration2D3D::metric() const { return _metric; }

void GrangeatRegistration2D3D::setMetric(const imgproc::AbstractErrorMetric* metric)
{
    if(metric == nullptr)
        qWarning("A nullptr to AbstractErrorMetric is ignored.");
    else
        _metric = metric;
}

float GrangeatRegistration2D3D::subSamplingLevel() const { return _subSamplingLevel; }

void GrangeatRegistration2D3D::setSubSamplingLevel(float subSamplingLevel)
{
    _subSamplingLevel = subSamplingLevel;
}

namespace {

float computeDeltaS(const OCL::VolumeResampler& volIntermedFct, const mat::ProjectionMatrix& pMat)
{
    Q_ASSERT(volIntermedFct.volDim().z > 1);
    const auto& dRange = volIntermedFct.rangeDim3();
    const auto deltaD = (dRange.end() - dRange.start()) / (volIntermedFct.volDim().z - 1);
    const auto magnification = float(pMat.magnificationX() + pMat.magnificationY()) * 0.5f;
    return magnification * deltaD;
}

double objFct(const std::vector<double>& x, std::vector<double>&, void* data)
{
    // x = [r_ t_] = [rx ry rz tx ty tz]
    const auto* d = static_cast<DataForOptimization*>(data);
    const auto H = Homography3D(rotationMatrix({ x[0], x[1], x[2] }), { x[3], x[4], x[5] });
    const auto& transformedBuf = d->radon3DCoordTransform.transform(H);
    const IntermediateFctPair intermPair(d->projIntermedFct,
                                         d->volumeIntermedResampler.sample(transformedBuf),
                                         IntermediateFctPair::VolumeDomain);
    return intermPair.inconsistency(d->metric);
}

Matrix3x3 rotationMatrix(const Vector3x1& axis)
{
    // optimization uses deg internally for rotational degrees of freedom (since "deg ~ mm")
    return mat::rotationMatrix(axis * (PI / 180.0));
}

void generateMsg(nlopt::result errCode)
{
    if(errCode < 0)
        qCritical("Optimization failed:");
    else if(errCode == nlopt::result::MAXEVAL_REACHED || errCode == nlopt::result::MAXTIME_REACHED)
        qWarning("Potential unintended termination of optimization (not converged).");

    switch (errCode)
    {
    // errors
    case nlopt::result::FAILURE:
        qCritical("Generic failure code.");
        break;
    case nlopt::result::INVALID_ARGS:
        qCritical("Invalid arguments (e.g. lower bounds are bigger than upper bounds, an unknown "
                  "algorithm was specified, etcetera).");
        break;
    case nlopt::result::OUT_OF_MEMORY:
        qCritical("Ran out of memory.");
        break;
    case nlopt::result::ROUNDOFF_LIMITED:
        qCritical("Halted because roundoff errors limited progress. (In this case, the "
                  "optimization still typically returns a useful result.)");
        break;
    case nlopt::result::FORCED_STOP:
        qCritical("Halted because of a forced termination: the user called nlopt_force_stop(opt) "
                  "on the optimization’s nlopt_opt object opt from the user’s objective function "
                  "or constraints.");
        break;

    // Successful termination
    case nlopt::result::SUCCESS:
        break;

    case nlopt::result::STOPVAL_REACHED:
        qDebug("Optimization stopped because stopval was reached.");
        break;

    case nlopt::result::FTOL_REACHED:
        qDebug("Optimization stopped because ftol_rel or ftol_abs was reached.");
        break;

    case nlopt::result::XTOL_REACHED:
        qDebug("Optimization stopped because xtol_rel or xtol_abs was reached.");
        break;

    case nlopt::result::MAXEVAL_REACHED:
        qDebug("Optimization stopped because maxeval was reached.");
        break;

    case nlopt::result::MAXTIME_REACHED:
        qDebug("Optimization stopped because maxtime was reached.");
        break;

    default:
        break;
    }
}

} // unnamed namespace
} // namespace NLOPT
} // namespace CTL
