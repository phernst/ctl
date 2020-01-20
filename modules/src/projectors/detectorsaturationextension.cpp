#include "detectorsaturationextension.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"

#include <future>

namespace CTL {

void DetectorSaturationExtension::configure(const AcquisitionSetup& setup,
                                            const AbstractProjectorConfig& config)
{
    _setup = setup;

    if(_nbSamples == 0)
        _nbSamples = _setup.system()->source()->spectrumDiscretizationHint();

    ProjectorExtension::configure(setup, config);
}

bool DetectorSaturationExtension::isLinear() const { return false; }

void DetectorSaturationExtension::setIntensitySampling(uint nbSamples) { _nbSamples = nbSamples; }

ProjectionData DetectorSaturationExtension::extendedProject(const MetaProjector& nestedProjector)
{
    auto ret = nestedProjector.project();

    auto saturationModelType = _setup.system()->detector()->saturationModelType();

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
        qWarning() << "DetectorSaturationExtension::project(): Undefined saturation model!";
        break;
    }

    return ret;
}

void DetectorSaturationExtension::processCounts(ProjectionData& projections)
{
    auto saturationModel = _setup.system()->detector()->saturationModel();

    auto processView = [saturationModel](SingleViewData* view, const std::vector<float>& n0) {
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

                ++mod;
            }
        }
    };

    std::vector<std::future<void>> futures;
    futures.reserve(_setup.nbViews());
    int v = 0;

    for(auto& view : projections.data())
    {
        _setup.prepareView(v++);
        futures.push_back(std::async(processView, &view, _setup.system()->photonsPerPixel()));
    }
}

void DetectorSaturationExtension::processExtinctions(ProjectionData& projections)
{
    auto saturationModel = _setup.system()->detector()->saturationModel();

    auto processView = [saturationModel](SingleViewData* view) {
        for(auto& module : view->data())
            for(auto& pix : module.data())
                pix = saturationModel->valueAt(pix);
    };

    std::vector<std::future<void>> futures;
    futures.reserve(_setup.nbViews());
    for(auto& view : projections.data())
        futures.push_back(std::async(processView, &view));
}

void DetectorSaturationExtension::processIntensities(ProjectionData& projections)
{
    auto saturationModel = _setup.system()->detector()->saturationModel();
    auto sourcePtr = _setup.system()->source();

    auto processView = [saturationModel](SingleViewData* view, const std::vector<float>& i0) {
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

                ++mod;
            }
        }
    };

    std::vector<std::future<void>> futures;
    futures.reserve(_setup.nbViews());
    int v = 0;
    std::vector<float> i0(_setup.system()->detector()->nbDetectorModules());

    for(auto& view : projections.data())
    {
        _setup.prepareView(v++);
        auto n0 = _setup.system()->photonsPerPixel();
        auto meanEnergy = sourcePtr->spectrum(_nbSamples).centroid();
        std::transform(n0.begin(), n0.end(), i0.begin(),
                       [meanEnergy](float count) { return count * meanEnergy; });

        futures.push_back(std::async(processView, &view, i0));
    }
}

} // namespace CTL
