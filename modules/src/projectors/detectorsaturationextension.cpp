#include "detectorsaturationextension.h"
#include "acquisition/radiationencoder.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"
#include "processing/threadpool.h"

#include <thread>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(DetectorSaturationExtension)

void DetectorSaturationExtension::configure(const AcquisitionSetup& setup)
{
    _setup = setup;

    ProjectorExtension::configure(setup);
}

/*!
 * Constructs a DetectorSaturationExtension and sets the number of samples used when (internally)
 * a spectrum needs to be sampled to \a nbSpectralSamples.
 */
DetectorSaturationExtension::DetectorSaturationExtension(uint nbSpectralSamples)
    : _nbSamples(nbSpectralSamples)
{
}

/*!
 * Returns false, because in general, detector saturation effects are non-linear.
 */
bool DetectorSaturationExtension::isLinear() const { return false; }

/*!
 * Sets the number of samples used when (internally) a spectrum needs to be sampled to 
 * \a nbSpectralSamples. This has no meaning if the saturation model set for the detector is
 * specified in the extinction domain (because processing extinctions does not require computation
 * of a spectrum at any point).
 * 
 * If \a nbSamples = 0, the sampling hint from the source component will be used (see 
 * AbstractSource::spectrumDiscretizationHint().
 */
void DetectorSaturationExtension::setIntensitySampling(uint nbSamples) { _nbSamples = nbSamples; }

// Use SerializationInterface::toVariant() documentation.
QVariant DetectorSaturationExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "DetectorSaturationExtension");

    return ret;
}

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with one key-value-pair: ("Intensity sampling points", _nbSamples),
 * which represents the number of sampling points used when (internally) a spectrum needs to be
 * sampled.
 *
 * This method is used within toVariant() to serialize the object's settings.
 */
QVariant DetectorSaturationExtension::parameter() const
{
    QVariantMap ret = ProjectorExtension::parameter().toMap();

    ret.insert("Intensity sampling points", _nbSamples);

    return ret;
}

// Use AbstractProjector::setParameter() documentation.
void DetectorSaturationExtension::setParameter(const QVariant& parameter)
{
    ProjectorExtension::setParameter(parameter);

    QVariantMap map = parameter.toMap();

    _nbSamples = map.value("Intensity sampling points", 0u).toUInt();
}

/*!
 * Performs the extension of the projection step by means of postprocessing the projection data
 * created by \a nestedProjector. This allows for handling the cases of single and composite 
 * volumes in the same fashion.
 */
ProjectionData DetectorSaturationExtension::extendedProject(const MetaProjector& nestedProjector)
{
    auto ret = nestedProjector.project();

    emit notifier()->information("Processing detector saturation.");

    auto saturationModelType = _setup.system()->detector()->saturationModelType();

    if(_nbSamples == 0)
        _nbSamples = _setup.system()->source()->spectrumDiscretizationHint();

    switch(saturationModelType)
    {
    case AbstractDetector::Extinction:
        processExtinctions(ret);
        break;
    case AbstractDetector::PhotonCount:
        processCounts(ret);
        break;
    case AbstractDetector::Intensity:
        processIntensities(ret);
        break;
    case AbstractDetector::Undefined:
        qWarning() << "DetectorSaturationExtension::project(): Undefined saturation model. "
                      "Extension has no effect!";
        break;
    }

    return ret;
}

/*!
 * Applies the detector saturation model to \a projections in the photon count domain.
 * Transformation of input extinction data from \a projections to counts is based on the incident
 * photon count queried from the system (see SimpleCTsystem::photonsPerPixel()).
 */
void DetectorSaturationExtension::processCounts(ProjectionData& projections)
{
    auto detectorPtr = _setup.system()->detector();

    auto processView = [detectorPtr](SingleViewData* view, std::vector<float> n0) {
        const auto saturationModel = detectorPtr->saturationModel();
        float count;
        uint mod = 0;
        for(auto& module : view->data())
        {
            for(auto& pix : module.data())
            {
                // transform extinction to photon count
                count = n0[mod] * std::exp(-pix);
                // pass intensity through saturation model
                count = saturationModel->valueAt(count);
                // back-transform to extinction and overwrite projection pixel value
                pix = std::log(n0[mod] / count);

            }
            ++mod;
        }
    };

    ThreadPool tp;

    auto v = 0u;
    for(auto& view : projections.data())
    {
        _setup.prepareView(v++);
        tp.enqueueThread(processView, &view, _setup.system()->photonsPerPixel());
    }
}

/*!
 * Applies the detector saturation model to \a projections in the extinction domain.
 */
void DetectorSaturationExtension::processExtinctions(ProjectionData& projections)
{
    auto detectorPtr = _setup.system()->detector();

    auto processView = [detectorPtr](SingleViewData* view) {
        const auto saturationModel = detectorPtr->saturationModel();
        for(auto& module : view->data())
            for(auto& pix : module.data())
                pix = saturationModel->valueAt(pix);
    };

    ThreadPool tp;

    auto v = 0u;
    for(auto& view : projections.data())
    {
        _setup.prepareView(v++); // only required if saturation model is volatile
        tp.enqueueThread(processView, &view);
    }
}

/*!
 * Applies the detector saturation model to \a projections in the intensity domain.
 * Transformation of input extinction data from \a projections to intensities is based on the
 * incident photon count queried from the system (see SimpleCTsystem::photonsPerPixel()) multiplied
 * by the mean energy in the incident X-ray spectrum. Note that this is an approximation. Ideally,
 * the final radiation spectrum reaching the detector would be considered instead.
 */
void DetectorSaturationExtension::processIntensities(ProjectionData& projections)
{
    auto detectorPtr = _setup.system()->detector();

    auto processView = [detectorPtr](SingleViewData* view, std::vector<float> i0) {
        const auto saturationModel = detectorPtr->saturationModel();
        float intensity;
        uint mod = 0;
        for(auto& module : view->data())
        {
            for(auto& pix : module.data())
            {
                // transform extinction to intensity
                intensity = i0[mod] * std::exp(-pix);
                // pass intensity through saturation model
                intensity = saturationModel->valueAt(intensity);
                // back-transform to extinction and overwrite projection pixel value
                pix = std::log(i0[mod] / intensity);
            }
            ++mod;
        }
    };

    ThreadPool tp;

    uint v = 0;
    std::vector<float> i0(detectorPtr->nbDetectorModules());
    RadiationEncoder enc(_setup.system());

    for(auto& view : projections.data())
    {
        _setup.prepareView(v++);
        const auto n0 = enc.photonsPerPixel();
        const auto meanEnergy = enc.finalSpectrum(_nbSamples).centroid();
        std::transform(n0.begin(), n0.end(), i0.begin(),
                       [meanEnergy](float count) { return count * meanEnergy; });

        tp.enqueueThread(processView, &view, i0);
    }
}

} // namespace CTL
