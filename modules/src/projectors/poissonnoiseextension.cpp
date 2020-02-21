#include "poissonnoiseextension.h"
#include "acquisition/geometryencoder.h"
#include "acquisition/radiationencoder.h"
#include "components/genericsource.h"
#include "img/chunk2d.h"

#include <future>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(PoissonNoiseExtension)

void PoissonNoiseExtension::configure(const AcquisitionSetup& setup)
{
    _setup = setup;

    ProjectorExtension::configure(setup);
}

ProjectionData PoissonNoiseExtension::extendedProject(const MetaProjector& nestedProjector)
{
    // compute (clean) projections
    auto ret = nestedProjector.project();

    if(!_useFixedSeed)
    {
        std::random_device rd;
        _seed = rd();
        _rng.seed(_seed);
    }

    auto const seed = static_cast<uint>(_rng());

    // add noise
    std::launch launchMode = std::launch::deferred;
    if(_useParallelization)
        launchMode |= std::launch::async;

    RadiationEncoder radiationEnc(_setup.system());
    std::vector<std::future<void>> futures(ret.nbViews());
    for(uint view = 0; view < ret.nbViews(); ++view)
    {
        _setup.prepareView(view);

        futures[view] = std::async(launchMode, processViewCompact, std::ref(ret.view(view)),
                                   radiationEnc.photonsPerPixel(), seed + view);
    }

    for(const auto& future : futures)
        future.wait();

    return ret;
}

bool PoissonNoiseExtension::isLinear() const { return false; }

// Use SerializationInterface::toVariant() documentation.
QVariant PoissonNoiseExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "PoissonNoiseExtension");

    return ret;
}

QVariant PoissonNoiseExtension::parameter() const
{
    QVariantMap ret = ProjectorExtension::parameter().toMap();

    qInfo() << _seed;

    ret.insert("Use fixed seed", _useFixedSeed);
    ret.insert("Use parallelization", _useParallelization);
    ret.insert("Seed", _seed);

    return ret;
}

void PoissonNoiseExtension::setParameter(const QVariant& parameter)
{
    qInfo() << "set parameter";

    ProjectorExtension::setParameter(parameter);

    QVariantMap map = parameter.toMap();
    _useFixedSeed = map.value("Use fixed seed", false).toBool();
    _useParallelization = map.value("Use parallelization", true).toBool();
    _seed = map.value("Seed", 0).toUInt();
}

void PoissonNoiseExtension::setFixedSeed(uint seed)
{
    _useFixedSeed = true;
    _seed = seed;
    _rng.seed(seed);
}

void PoissonNoiseExtension::setRandomSeedMode() { _useFixedSeed = false; }

void PoissonNoiseExtension::setParallelizationEnabled(bool enabled)
{
    _useParallelization = enabled;
}

void PoissonNoiseExtension::processViewCompact(SingleViewData& view,
                                               const std::vector<float>& i_0,
                                               uint seed)
{
    if(qFuzzyIsNull(std::accumulate(i_0.cbegin(), i_0.cend(), 0.0f)))
    {
        qDebug() << "PoissonNoiseExtension::processViewCompact(): Skipped view with i_0 = 0.";
        return;
    }

    std::mt19937_64 gen(seed);
    constexpr float gaussianThreshold = 1.0e4; // threshold for switch to normal distribution

    auto randomizedPoisson = [&gen] (float count) // Poisson distributed count
    {
        std::poisson_distribution<ulong> d(count);
        return float(d(gen));
    };

    auto randomizedGaussian = [&gen] (float count) // Normally distributed count
    {
        std::normal_distribution<float> d(count, std::sqrt(count));
        return d(gen);
    };

    float origCount, noisyCount;
    uint mod = 0u;
    for(auto& module : view.data())
    {
        for(auto& pix : module.data())
        {
            origCount = i_0[mod] * std::exp(-pix); // mean

            // randomize counts
            if(origCount < gaussianThreshold)
                noisyCount = randomizedPoisson(origCount);
            else // good approximation for large counts
                noisyCount = randomizedGaussian(origCount);

            pix = std::log(i_0[mod] / noisyCount);
        }
        ++mod;
    }
}

} // namespace CTL
