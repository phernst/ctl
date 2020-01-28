#include "detectorsaturationextension.h"
#include "acquisition/radiationencoder.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"

#include <thread>

static const uint OPTIMAL_NB_THREADS = std::max({ 1u, std::thread::hardware_concurrency() });

namespace  {
class ThreadPool
{
public:
    ThreadPool(size_t nbThreads = std::thread::hardware_concurrency())
        : _pool(nbThreads == 0 ? 1 : nbThreads)
        , _curThread(_pool.begin())
    {
    }

    ~ThreadPool()
    {
        std::for_each(_pool.begin(), _pool.end(), [](std::thread& t){ if(t.joinable()) t.join(); });
    }

    // delete copy
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator= (const ThreadPool&) = default;
    // default move
    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator= (ThreadPool&&) = default;

    template <class Function, class... Args>
    void enqueueThread(Function&& f, Args&&... args)
    {
        if(_curThread->joinable())
            _curThread->join();

        *_curThread = std::thread(std::forward<Function>(f),
                                  std::forward<Args>(args)...);

        ++_curThread;
        if(_curThread == _pool.end())
            _curThread = _pool.begin();
    }

private:
    std::vector<std::thread> _pool;
    std::vector<std::thread>::iterator _curThread;
};
} // unnamed namespace

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
    ThreadPool tp(nbThreads);

    auto v = 0u;
    for(auto& view : projections.data())
    {
        _setup.prepareView(v++);
        tp.enqueueThread(processView, &view, _setup.system()->photonsPerPixel());
    }
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
    ThreadPool tp(nbThreads);

    auto v = 0u;
    for(auto& view : projections.data())
    {
        _setup.prepareView(v++); // only required if saturation model is volatile
        tp.enqueueThread(processView, &view);
    }
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
    ThreadPool tp(nbThreads);

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
