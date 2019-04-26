#include "poissonnoiseextension.h"
#include "components/genericsource.h"
#include "acquisition/geometryencoder.h"
#include "img/chunk2d.h"

#include <future>

namespace CTL {

void PoissonNoiseExtension::configure(const AcquisitionSetup &setup,
                                      const AbstractProjectorConfig &config)
{
    _setup = setup;
    _config.reset(config.clone());

    ProjectorExtension::configure(setup, config);
}

ProjectionData PoissonNoiseExtension::extendedProject(const MetaProjector& nestedProjector)
{
    // compute (clean) projections
    auto ret = nestedProjector.project();

    if(!_useFixedSeed)
    {
        std::random_device rd;
        _rng.seed(rd());
    }

    uint seed = _rng();

    // add noise
    std::launch launchMode = std::launch::deferred;
    if(_useParallelization)
        launchMode |= std::launch::async;

    std::vector<std::future<void>> futures(ret.nbViews());
    for(uint view = 0; view < ret.nbViews(); ++view)
    {
        _setup.prepareView(view);

        futures[view] = std::async(launchMode, processViewCompact, &ret.view(view),
                                   _setup.system()->photonsPerPixel(), seed + view);
    }

    for(const auto& future : futures)
        future.wait();

    return ret;
}

bool PoissonNoiseExtension::isLinear() const { return false; }

void PoissonNoiseExtension::setFixedSeed(uint seed)
{
    _useFixedSeed = true;
    _rng.seed(seed);
}

void PoissonNoiseExtension::setRandomSeedMode() { _useFixedSeed = false; }

void PoissonNoiseExtension::setParallelizationEnabled(bool enabled)
{
    _useParallelization = enabled;
}

void PoissonNoiseExtension::processViewCompact(SingleViewData* view, std::vector<float> i_0, uint seed)
{
    if(qFuzzyIsNull(std::accumulate(i_0.begin(), i_0.end(), 0.0)))
    {
        qDebug() << "PoissonNoiseExtension::processViewCompact(): Skipped view with i_0 = 0.";
        return;
    }

    std::mt19937_64 gen(seed);

    uint mod = 0;
    for(auto& module : view->data())
    {
        for(auto& pix : module.data())
        {
            auto origCount = i_0[mod] * expf(-pix); // mean
            std::poisson_distribution<ulong> d(origCount);
            auto noisyCount = d(gen); // Poisson distributed random number
            pix = logf(float(i_0[mod]) / float(noisyCount));
        }
        ++mod;
    }
}

}
