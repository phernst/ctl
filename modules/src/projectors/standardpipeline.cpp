#include "standardpipeline.h"

#include "raycasterprojector.h"
#include "arealfocalspotextension.h"
#include "detectorsaturationextension.h"
#include "poissonnoiseextension.h"
#include "spectraleffectsextension.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(StandardPipeline)

/*!
 * Constructs a StandardPipeline object and with the ApproximationPolicy \a policy.
 *
 * The default configuration enables spectral effects and Poisson noise simulation.
 */
StandardPipeline::StandardPipeline(ApproximationPolicy policy)
    : _projector (new OCL::RayCasterProjector)
    , _extAFS (new ArealFocalSpotExtension)
    , _extDetSat (new DetectorSaturationExtension)
    , _extPoisson (new PoissonNoiseExtension)
    , _extSpectral (new SpectralEffectsExtension)
    , _approxMode(policy)
{
    _pipeline.setProjector(_projector);

    // configure extensions
    _extAFS->setDiscretization( {3, 3} );
    if(policy == ApproximationPolicy::Full_Approximation)
        _extAFS->enableLowExtinctionApproximation();

    enableArealFocalSpot(false);
    enableDetectorSaturation(false);
    enablePoissonNoise(true);
    enableSpectralEffects(true);
}

StandardPipeline::~StandardPipeline()
{
    // handover ownership to ProjectionPipeline member
    enableArealFocalSpot(true);
    enableDetectorSaturation(true);
    enablePoissonNoise(true);
    enableSpectralEffects(true);
}

/*!
 * \brief Sets the acquisition setup for the simulation to \a setup.
 *
 * Sets the acquisition setup for the simulation to \a setup. This needs to be done prior to calling project().
 */
void StandardPipeline::configure(const AcquisitionSetup& setup)
{
    _pipeline.configure(setup);
}

/*!
 * \brief Creates projection data from \a volume.
 *
 * Creates projection data from \a volume using the current processing pipeline configuration of
 * this instance. Uses the last acquisition setup set by configure().
 */
ProjectionData StandardPipeline::project(const VolumeData& volume)
{
    return _pipeline.project(volume);
}

/*!
 * \brief Creates projection data from the composite volume \a volume.
 *
 * Creates projection data from the composite volume \a volume using the current processing pipeline
 * configuration of this instance. Uses the last acquisition setup set by configure().
 */
ProjectionData StandardPipeline::projectComposite(const CompositeVolume& volume)
{
    return _pipeline.projectComposite(volume);
}

/*!
 * Returns true if the application of the full processing pipeline is linear.
 */
bool StandardPipeline::isLinear() const
{
    return _pipeline.isLinear();
}

ProjectorNotifier* StandardPipeline::notifier()
{
    return _pipeline.notifier();
}

void StandardPipeline::fromVariant(const QVariant& variant)
{
    AbstractProjector::fromVariant(variant);

    QVariantMap map = variant.toMap();

    enableArealFocalSpot(false);
    enableDetectorSaturation(false);
    enablePoissonNoise(false);
    enableSpectralEffects(false);

    _pipeline.setProjector(SerializationHelper::parseProjector(map.value("projector")));

    _extAFS->fromVariant(map.value("ext AFS"));
    _extDetSat->fromVariant(map.value("ext DetSat"));
    _extPoisson->fromVariant(map.value("ext Poisson"));
    _extSpectral->fromVariant(map.value("ext spectral"));

    _approxMode = ApproximationPolicy(map.value("approximation policy",
                                                int(ApproximationPolicy::Default_Approximation)).toInt());

    enableArealFocalSpot(map.value("use areal focal spot").toBool());
    enableDetectorSaturation(map.value("use detector saturation").toBool());
    enablePoissonNoise(map.value("use poisson noise").toBool());
    enableSpectralEffects(map.value("use spectral effects").toBool());
}

QVariant StandardPipeline::toVariant() const
{
    QVariantMap ret = AbstractProjector::toVariant().toMap();

    ret.insert("#", "StandardPipeline");
    ret.insert("use areal focal spot", _arealFSEnabled);
    ret.insert("use detector saturation", _detSatEnabled);
    ret.insert("use poisson noise", _poissonEnabled);
    ret.insert("use spectral effects", _spectralEffEnabled);
    ret.insert("approximation policy", _approxMode);

    ret.insert("projector", _projector->toVariant());
    ret.insert("ext AFS", _extAFS->toVariant());
    ret.insert("ext DetSat", _extDetSat->toVariant());
    ret.insert("ext Poisson", _extPoisson->toVariant());
    ret.insert("ext spectral", _extSpectral->toVariant());

    return ret;
}

/*!
 * Enables/disables the simulation of areal focal spot effects, according to \a enable.
 */
void StandardPipeline::enableArealFocalSpot(bool enable)
{
    if(enable == _arealFSEnabled) // no change
        return;

    if(enable) // insert AFS into pipeline (first position)
        _pipeline.insertExtension(posAFS(), _extAFS);
    else // remove AFS from pipeline (first position)
        _pipeline.releaseExtension(posAFS());

    _arealFSEnabled = enable;
}

/*!
 * Enables/disables the simulation of detector saturation effects, according to \a enable.
 *
 * This only has an effect on the simulation if the detector component of the system passed with
 * the setup during configure() has a detector response model (see AbstractDetector::setSaturationModel()).
 */
void StandardPipeline::enableDetectorSaturation(bool enable)
{
    if(enable == _detSatEnabled) // no change
        return;

    if(enable) // insert det. sat. into pipeline (last position)
        _pipeline.appendExtension(_extDetSat);
    else // remove det. sat. from pipeline (last position)
        _pipeline.releaseExtension(_pipeline.nbExtensions() - 1u);

    _detSatEnabled = enable;
}

/*!
 * Enables/disables the simulation of Poisson noise, according to \a enable.
 */
void StandardPipeline::enablePoissonNoise(bool enable)
{
    if(enable == _poissonEnabled) // no change
        return;

    if(enable) // insert Poisson into pipeline (after AFS and spectral)
        _pipeline.insertExtension(posPoisson(), _extPoisson);
    else // remove Poisson from pipeline (after AFS and spectral)
        _pipeline.releaseExtension(posPoisson());

    _poissonEnabled = enable;
}

/*!
 * Enables/disables the simulation of spectral effects, according to \a enable.
 *
 * Spectral effects require full spectral information (see SpectralVolumeData) in the volume data
 * passed to project(). Otherwise, the spectral effects step will be skipped.
 *
 * Spectral detector response effects will be considered if a corresponding response model has been
 * set to the detector component (see AbstractDetector::setSpectralResponseModel()) of the system
 * passed with the setup during configure(). Note that trying to simulate settings with a spectral
 * response model in combination with volume data without full spectral information is not supported
 * and leads to an exception.
 */
void StandardPipeline::enableSpectralEffects(bool enable)
{
    if(enable == _spectralEffEnabled) // no change
        return;

    if(enable) // insert spectral ext. into pipeline (after AFS)
        _pipeline.insertExtension(posSpectral(), _extSpectral);
    else // remove Poisson from pipeline (after AFS)
        _pipeline.releaseExtension(posSpectral());

    _spectralEffEnabled = enable;
}

/*!
 * Returns a handle to the settings for the areal focal spot simulation.
 *
 * Areal focal spot settings are:
 * - setDiscretization(const QSize& discretization): sets the number of sampling
 * points for the subsampling of the areal focal spot to \a discretization (width x height)
 * [ default value: {3, 3} ]
 * - enableLowExtinctionApproximation(bool enable): sets the use of the linear approximation to
 * \a enable. [ default: \c false (\c true for StandardPipeline::Full_Approximation) ]
 *
 * Example:
 * \code
 * StandardPipeline pipe;
 * pipe.enableArealFocalSpot();
 * pipe.settingsArealFocalSpot().setDiscretization( { 2, 2 } );
 * \endcode
 *
 * \sa ArealFocalSpotExtension::setDiscretization()
 */
StandardPipeline::SettingsAFS StandardPipeline::settingsArealFocalSpot()
{
    return { *_extAFS };
}

/*!
 * Returns a handle to the settings for the detector saturation simulation.
 *
 * Detector saturation settings are:
 * - setSpectralSamples(uint nbSamples): sets the number of energy bins used to sample the spectrum
 * when processing intensity saturation to \a nbSamples [ default value: 0 (i.e. use sampling hint
 * of source component) ]
 *
 * Example:
 * \code
 * StandardPipeline pipe;
 * pipe.enableDetectorSaturation();
 * pipe.settingsDetectorSaturation().setSpectralSamples(10);
 * \endcode
 *
 * \sa DetectorSaturationExtension::setIntensitySampling()
 */
StandardPipeline::SettingsDetectorSaturation StandardPipeline::settingsDetectorSaturation()
{
    return { *_extDetSat };
}

/*!
 * Returns a handle to the settings for the Poisson noise simulation.
 *
 * Poisson noise settings are:
 * - setFixedSeed(uint seed): sets a fixed seed for the pseudo random number generation [ default
 * value: not used]
 * - setRandomSeedMode(): (re-)enables the random seed mode, any fixed seed set will be ignored
 * until setFixedSeed() is called again [ default: random seed mode used ]
 * - setParallelizationMode(bool enabled): sets the use of parallelization to \a enabled [ default
 * value: true (i.e. parallelization enabled) ]
 *
 * Example:
 * \code
 * StandardPipeline pipe;
 * pipe.settingsPoissonNoise().setFixedSeed(1337);
 * pipe.settingsPoissonNoise().setParallelizationMode(false);
 * \endcode
 *
 * \sa PoissonNoiseExtension::setFixedSeed(), PoissonNoiseExtension::setRandomSeedMode(),
 * PoissonNoiseExtension::setParallelizationEnabled()
 */
StandardPipeline::SettingsPoissonNoise StandardPipeline::settingsPoissonNoise()
{
    return { *_extPoisson };
}

/*!
 * Returns a handle to the settings for the spectral effects simulation.
 *
 * Spectral effects settings are:
 * - setSamplingResolution(float energyBinWidth): sets the energy bin width used to sample the
 * spectrum to \a energyBinWidth (in keV) [ default value: 0 (i.e. resolution determined
 * automatically based on sampling hint of source component) ]
 *
 * Example:
 * \code
 * StandardPipeline pipe;
 * pipe.settingsSpectralEffects().setSamplingResolution(5.0f);
 * \endcode
 *
 * \sa SpectralEffectsExtension::setSpectralSamplingResolution()
 */
StandardPipeline::SettingsSpectralEffects StandardPipeline::settingsSpectralEffects()
{
    return { *_extSpectral };
}

/*!
 * Returns a handle to the settings for the ray caster projector.
 *
 * Ray caster settings are:
 * - setInterpolation(bool enabled): sets the use of interpolation in the OpenCL kernel to
 * \a enabled; disable interpolation when your OpenCL device does not have image support
 * [ default value: true (i.e. interpolation enabled) ]
 * - setRaysPerPixel(const QSize& sampling): sets the number of rays cast per pixel to \a sampling
 * (width x height) [ default value: {1, 1} ]
 * - setRaySampling(float sampling): sets the step length used to traverse the ray to \a sampling,
 * which is defined as the fraction of the length of a voxel in its shortest dimension [ default
 * value: 0.3 ]
 * - setVolumeUpSampling(uint upSamplingFactor): sets the factor for upsampling of the input volume
 * data to \a upSamplingFactor [ default value: 1 (i.e. no upsampling) ]
 *
 * Example:
 * \code
 * StandardPipeline pipe;
 * pipe.settingsRayCaster().setRaysPerPixel( { 2, 2 } );
 * pipe.settingsRayCaster().setVolumeUpSampling(2);
 * \endcode
 *
 * \sa OCL::RayCasterProjector::settings()
 */
StandardPipeline::SettingsRayCaster StandardPipeline::settingsRayCaster() {
    return { *_projector };
}

// ###############
// private methods
// ###############

/*!
 * Returns the position of the areal focal spot extension in the standard pipeline.
 * This is defined to always be the first position (maximum efficiency).
 */
uint StandardPipeline::posAFS() const
{
    return 0;
}

/*!
 * Returns the position of the detector saturation extension in the standard pipeline.
 * This is defined to always be the last position, as it is only accurate in this spot.
 */
uint StandardPipeline::posDetSat() const
{
    return uint(_arealFSEnabled)
            + uint(_spectralEffEnabled)
            + uint(_poissonEnabled);
}

/*!
 * Returns the position of the Poisson noise extension in the standard pipeline.
 * Depending on whether the mode has been set to StandardPipeline::No_Approximation or not (i.e.
 * StandardPipeline::Full_Approximation or StandardPipeline::Default_Approximation), the
 * Poisson extension is placed before or after the spectral effects extension, respectively.
 */
uint StandardPipeline::posPoisson() const
{
    return (_approxMode == No_Approximation) ? uint(_arealFSEnabled)
                                             : uint(_arealFSEnabled) + uint(_spectralEffEnabled);
}

/*!
 * Returns the position of the spectral effects extension in the standard pipeline.
 * Depending on whether the mode has been set to StandardPipeline::No_Approximation or not (i.e.
 * StandardPipeline::Full_Approximation or StandardPipeline::Default_Approximation), the
 * spectral effects extension is placed after or before the Poisson noise extension, respectively.
 */
uint StandardPipeline::posSpectral() const
{
    return (_approxMode == No_Approximation) ? uint(_arealFSEnabled) + uint(_poissonEnabled)
                                             : uint(_arealFSEnabled);
}

void StandardPipeline::SettingsPoissonNoise::setFixedSeed(uint seed)
{
    _ext.setFixedSeed(seed);
}

void StandardPipeline::SettingsPoissonNoise::setRandomSeedMode()
{
    _ext.setRandomSeedMode();
}

void StandardPipeline::SettingsPoissonNoise::setParallelizationMode(bool enabled)
{
    _ext.setParallelizationEnabled(enabled);
}

void StandardPipeline::SettingsAFS::setDiscretization(const QSize &discretization)
{
    _ext.setDiscretization(discretization);
}

void StandardPipeline::SettingsAFS::enableLowExtinctionApproximation(bool enable)
{
    _ext.enableLowExtinctionApproximation(enable);
}

void StandardPipeline::SettingsDetectorSaturation::setSpectralSamples(uint nbSamples)
{
    _ext.setIntensitySampling(nbSamples);
}

void StandardPipeline::SettingsSpectralEffects::setSamplingResolution(float energyBinWidth)
{
    _ext.setSpectralSamplingResolution(energyBinWidth);
}

void StandardPipeline::SettingsRayCaster::setInterpolation(bool enabled)
{
    _proj.settings().interpolate = enabled;
}

void StandardPipeline::SettingsRayCaster::setRaysPerPixel(const QSize& sampling)
{
    _proj.settings().raysPerPixel[0] = static_cast<uint>(sampling.width());
    _proj.settings().raysPerPixel[1] = static_cast<uint>(sampling.height());
}

void StandardPipeline::SettingsRayCaster::setRaySampling(float sampling)
{
    _proj.settings().raySampling = sampling;
}

void StandardPipeline::SettingsRayCaster::setVolumeUpSampling(uint upSamplingFactor)
{
    _proj.settings().volumeUpSampling = upSamplingFactor;
}

/*!
 * \enum StandardPipeline::ApproximationPolicy
 * Enumeration for the approximation behavior in the standard pipeline. See Detailed Description
 * for more details.
 */

/*! \var StandardPipeline::ApproximationPolicy StandardPipeline::Full_Approximation,
 *  Same configuration as in Default_Approximation setting. Additionally, a linearized approach is
 *  used in the ArealFocalSpotExtension (if enabled).
 *  Not suited in combination with a spectral detector response and inaccurate in case of high
 *  extinction gradients (e.g. edges of highly absorbing material) in the projection images.
 */

/*! \var StandardPipeline::ApproximationPolicy StandardPipeline::Default_Approximation,
 *  The default setting for the StandardPipeline.
 *  Configuration in which Poisson noise addition is applied to final result of the spectral effects
 *  simulation. Approximation with substantially increased computation speed. Not suited in
 *  combination with a spectral detector response.
 */

/*! \var StandardPipeline::ApproximationPolicy StandardPipeline::No_Approximation
 *  Configuration with spectral effects simulation wrapping Poisson noise addition for each energy
 *  bin. Approximation-free but increased computation effort. Can be used in combination with a
 *  spectral detector response.
 */

} // namespace CTL
