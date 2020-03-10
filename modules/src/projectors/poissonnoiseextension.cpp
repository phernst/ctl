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

/*!
 * Constructs a PoissonNoiseExtension and sets the fixed seed for the random number generator to
 * \a fixedSeed.
 *
 * To use PoissonNoiseExtension without fixed seed, use the default constructor instead or
 * reactivate random seeding after construction with setRandomSeedMode().
 *
 * Optionally, parallelization can be deactivated when passing \a useParalellization = \c false.
 */
PoissonNoiseExtension::PoissonNoiseExtension(uint fixedSeed, bool useParalellization)
    : _useParallelization(useParalellization)
{
    setFixedSeed(fixedSeed);
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

/*!
 * Returns false, because addition of Poisson noise is non-linear (it operates on the count domain,
 * which involves exponentiation of the extinction values).
 */
bool PoissonNoiseExtension::isLinear() const { return false; }

// Use SerializationInterface::toVariant() documentation.
QVariant PoissonNoiseExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "PoissonNoiseExtension");

    return ret;
}

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with three key-value-pairs:
 * - ("Use fixed seed", _useFixedSeed), storing whether fixed seed mode is used or not.
 * - ("Use parallelization", _useParallelization), storing whether parallelization is used or not.
 * - ("Seed", _seed), storing the seed used for the RNG.
 *
 * This method is used within toVariant() to serialize the object's settings.
 */
QVariant PoissonNoiseExtension::parameter() const
{
    QVariantMap ret = ProjectorExtension::parameter().toMap();

    ret.insert("Use fixed seed", _useFixedSeed);
    ret.insert("Use parallelization", _useParallelization);
    ret.insert("Seed", _seed);

    return ret;
}

// Use AbstractProjector::setParameter() documentation.
void PoissonNoiseExtension::setParameter(const QVariant& parameter)
{
    ProjectorExtension::setParameter(parameter);

    QVariantMap map = parameter.toMap();
    _useFixedSeed = map.value("Use fixed seed", false).toBool();
    _useParallelization = map.value("Use parallelization", true).toBool();
    _seed = map.value("Seed", 0).toUInt();
}

/*!
 * Activates the fixed seed mode and sets the fixed seed to \a seed.
 *
 * To reactivate random seeding, use setRandomSeedMode().
 */
void PoissonNoiseExtension::setFixedSeed(uint seed)
{
    _useFixedSeed = true;
    _seed = seed;
    _rng.seed(seed);
}

/*!
 * Reactivates random seeding for the random number generator.
 *
 * Has no effect if fixed seeding has not been enabled before.
 */
void PoissonNoiseExtension::setRandomSeedMode() { _useFixedSeed = false; }

/*!
 * Sets the use of parallelization for the processing of multiple projections to \a enabled.
 */
void PoissonNoiseExtension::setParallelizationEnabled(bool enabled)
{
    _useParallelization = enabled;
}

/*!
 * The actual processing of projection data from a single view \a view (ie. with all its detector
 * modules).
 *
 * Transforms data into count domain based on initial counts passed by \a i_0.
 *
 * This method approximates the Poisson distribution by a normal distribution for counts larger than
 * 1.0e4.
 */
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
