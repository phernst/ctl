#include "detectorsaturationextension.h"
#include "acquisition/radiationencoder.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"

#include <thread>

static const uint OPTIMAL_NB_THREADS = std::max({ 1u, std::thread::hardware_concurrency() });

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

    const auto nbThreads = std::min({ _setup.nbViews(), OPTIMAL_NB_THREADS });
    std::vector<std::thread> threads(nbThreads);
    auto curThreadIdx = 0u;

    auto prepCurThread = [&threads, &curThreadIdx, nbThreads]() -> std::thread&
    {
        auto& curThread = threads[curThreadIdx];
        if(curThread.joinable())
            curThread.join();
        ++curThreadIdx;
        curThreadIdx = curThreadIdx % nbThreads;
        return curThread;
    };

    auto v = 0u;
    for(auto& view : projections.data())
    {
        _setup.prepareView(v++);
        prepCurThread() = std::thread(processView, &view, _setup.system()->photonsPerPixel());
    }

    // wait for finished
    std::for_each(threads.begin(), threads.end(), [](std::thread& t){ if(t.joinable()) t.join(); });
}

void DetectorSaturationExtension::processExtinctions(ProjectionData& projections)
{
    auto detectorPtr = _setup.system()->detector();

    auto processView = [detectorPtr](SingleViewData* view) {
        const auto saturationModel = detectorPtr->saturationModel();
        for(auto& module : view->data())
            for(auto& pix : module.data())
                pix = saturationModel->valueAt(pix);
    };

    const auto nbThreads = std::min({ _setup.nbViews(), OPTIMAL_NB_THREADS });
    std::vector<std::thread> threads(nbThreads);
    auto curThreadIdx = 0u;

    auto prepCurThread = [&threads, &curThreadIdx, nbThreads]() -> std::thread&
    {
        auto& curThread = threads[curThreadIdx];
        if(curThread.joinable())
            curThread.join();
        ++curThreadIdx;
        curThreadIdx = curThreadIdx % nbThreads;
        return curThread;
    };

    auto v = 0u;
    for(auto& view : projections.data())
    {
        _setup.prepareView(v++); // only required if saturation model is volatile
        prepCurThread() = std::thread(processView, &view);
    }

    // wait for finished
    std::for_each(threads.begin(), threads.end(), [](std::thread& t) { t.join(); });
}

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

    const auto nbThreads = std::min({ _setup.nbViews(), OPTIMAL_NB_THREADS });
    std::vector<std::thread> threads(nbThreads);
    auto curThreadIdx = 0u;

    auto prepCurThread = [&threads, &curThreadIdx, nbThreads]() -> std::thread&
    {
        auto& curThread = threads[curThreadIdx];
        if(curThread.joinable())
            curThread.join();
        ++curThreadIdx;
        curThreadIdx = curThreadIdx % nbThreads;
        return curThread;
    };

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

        prepCurThread() = std::thread(processView, &view, i0);
    }

    // wait for finished
    std::for_each(threads.begin(), threads.end(), [](std::thread& t) { t.join(); });
}

} // namespace CTL
