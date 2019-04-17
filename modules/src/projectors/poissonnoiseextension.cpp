#include "poissonnoiseextension.h"
#include "components/genericsource.h"
#include "img/chunk2d.h"

#include <random>
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

    // add noise
    if(_useParallelization)
    {
        std::random_device rd;
        std::vector<std::future<void>> futures(ret.nbViews());
        for(uint view = 0; view < ret.nbViews(); ++view)
        {
            _setup.prepareView(view);
            auto i_0 = _setup.system()->source()->photonFlux();
            uint seed = _useFixedSeed ? _fixedSeed + view : rd();
            futures[view] = std::async(processViewCompact,
                                       &ret.view(view), i_0, rd());
        }
        for(const auto& future : futures)
            future.wait();
    }
    else
    {
        uint seedShift = 0;
        for(uint view = 0; view < ret.nbViews(); ++view)
        {
            _setup.prepareView(view);
            auto i_0 = _setup.system()->source()->photonFlux();
            seedShift = std::max(seedShift, view * ret.view(view).nbModules());
            processView(ret.view(view), i_0, seedShift);
        }
    }

    return ret;
}

bool PoissonNoiseExtension::isLinear() const { return false; }

void PoissonNoiseExtension::setFixedSeed(uint seed) { _fixedSeed = seed; }

void PoissonNoiseExtension::setParallelizationEnabled(bool enabled)
{
    _useParallelization = enabled;
}

void PoissonNoiseExtension::toggleFixedSeed(bool enabled) { _useFixedSeed = enabled; }

void PoissonNoiseExtension::processView(SingleViewData &view, double i_0, uint seedShift)
{
    if(qFuzzyIsNull(i_0))
    {
        qDebug() << "PoissonNoiseExtension::processView(): Skipped view with i_0 = 0.";
        return;
    }

    std::random_device rd;
    for(uint mod = 0; mod < view.nbModules(); ++mod)
    {
        auto counts = transformedToCounts(view.module(mod), i_0);

        uint seed = _useFixedSeed ? _fixedSeed + mod + seedShift : rd();
        addNoiseToData(counts, seed);

        view.module(mod) = transformedToExtinctions(counts, i_0);
    }
}

void PoissonNoiseExtension::processViewCompact(SingleViewData* view, double i_0, uint seed)
{
    if(qFuzzyIsNull(i_0))
    {
        qDebug() << "PoissonNoiseExtension::processViewCompact(): Skipped view with i_0 = 0.";
        return;
    }

    std::mt19937_64 gen(seed);

    for(auto& module : view->data())
        for(auto& pix : module.data())
        {
            auto origCount = i_0 * double(exp(-pix)); // mean
            std::poisson_distribution<ulong> d(origCount);
            auto noisyCount = d(gen); // Poisson distributed random number
            pix = log(float(i_0) / float(noisyCount));
        }
}

Chunk2D<double> PoissonNoiseExtension::transformedToCounts(const SingleViewData::ModuleData &module, double i_0) const
{
    Chunk2D<double> ret(module.width(), module.height());
    ret.allocateMemory();

    auto rawDataPtr = ret.rawData();
    const auto& moduleDat = module.constData();
    for(uint pix = 0; pix < module.nbElements(); ++pix)
        rawDataPtr[pix] = i_0 * exp(-moduleDat[pix]);

    return ret;
}

SingleViewData::ModuleData PoissonNoiseExtension::transformedToExtinctions(const Chunk2D<double> &counts, double i_0) const
{
    SingleViewData::ModuleData ret(counts.width(), counts.height());
    ret.allocateMemory();

    auto rawDataPtr = ret.rawData();
    const auto& countsDat = counts.constData();
    for(uint pix = 0; pix < counts.nbElements(); ++pix)
        rawDataPtr[pix] = static_cast<float>(log(i_0 / countsDat[pix]));

    return ret;
}

void PoissonNoiseExtension::addNoiseToData(Chunk2D<double> &data, uint seed)
{
    std::mt19937_64 gen(seed);

    auto rawDataPtr = data.rawData();
    for(uint pix = 0; pix < data.nbElements(); ++pix)
    {
        std::poisson_distribution<ulong> d(rawDataPtr[pix]);

        rawDataPtr[pix] = static_cast<double>(d(gen));
    }
}

}
