#include "spectraleffectsextension.h"
#include "acquisition/preparesteps.h"
#include "acquisition/radiationencoder.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"
#include "models/stepfunctionmodels.h"
#include <limits>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(SpectralEffectsExtension)

/*!
 * Constructs a SpectralEffectsExtension and sets the bin width for sub-sampling of the spectral
 * range to \a energyBinWidth (in keV).
 */
SpectralEffectsExtension::SpectralEffectsExtension(float energyBinWidth)
    : _deltaE(energyBinWidth)
{
}

void SpectralEffectsExtension::configure(const AcquisitionSetup& setup)
{
    _setup = setup;

    updateSpectralInformation();

    ProjectorExtension::configure(setup);
}

/*!
 * Computes projections of \a volume considering spectral effects, by means of creating individual
 * projections for a number of energy bins and averaging the results in intensity domain.
 *
 * For a specific pixel, computation of the extinction value can be expressed as:
 *
 * \f$
 * \begin{align*}
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-m(E)\mathcal{F}_{\textrm{linear}}(\rho)\right]}\\
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-\mathcal{F}_{\textrm{non-linear}}(m(E)\cdot\rho)\right]},
 * \end{align*}
 * \f$
 *
 * depending on whether the nested projector of this instance is linear
 * (\f$\mathcal{F}_{\textrm{linear}}\f$, upper row) or not (\f$\mathcal{F}_{\textrm{non-linear}}\f$,
 * bottom row). Here, \f$i_{0}(E)\f$ denotes the initial intensity of energy \f$E\f$, \f$m(E)\f$
 * is the energy-dependent mass attenuation coefficient of the material, and \f$\rho\f$ is the
 * material density (3D voxelized data); I_{0} denotes the total intensity.
 * As can be seen from the equations, in case of a linear nested projector, it is sufficient to
 * forward project the material density, thus only a single projection operation is required.
 */
ProjectionData SpectralEffectsExtension::project(const VolumeData& volume)
{
    return projectComposite(CompositeVolume{ volume });
}

/*!
 * Computes projections of the composite volume \a volume considering spectral effects, by means of
 * creating individual projections for a number of energy bins and averaging the results in intensity
 * domain.
 *
 * For a specific pixel, computation of the extinction value can be expressed as:
 *
 * \f$
 * \begin{align*}
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-\sum_{k}m_{k}(E)\mathcal{F}_{\textrm{linear}}(\rho_{k})\right]}\\
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-\sum_{k}\mathcal{F}_{\textrm{non-linear}}(m_{k}(E)\cdot\rho_{k})\right]}\\
 * \end{align*}
 * \f$
 *
 * depending on whether the nested projector of this instance is linear
 * (\f$\mathcal{F}_{\textrm{linear}}\f$, upper row) or not (\f$\mathcal{F}_{\textrm{non-linear}}\f$,
 * bottom row). Here, \f$i_{0}(E)\f$ denotes the initial intensity of energy \f$E\f$,
 * \f$m_{k}(E)\f$ is the energy-dependent mass attenuation coefficient of material \f$k\f$, and
 * \f$\rho_{k}\f$ its material density (3D voxelized data); I_{0} denotes the total intensity.
 * As can be seen from the equations, in case of a linear nested projector, it is sufficient to
 * forward project the material densities of all materials, thus only a single projection operation
 * per sub-volume in \a volume is required. Note that this might result in substantial memory
 * requirement.
 */
ProjectionData SpectralEffectsExtension::projectComposite(const CompositeVolume& volume)
{
    if(canBypassExtension(volume))
    {
        qInfo() << "Bypassing SpectralEffectsExtension.";
        return ProjectorExtension::projectComposite(volume);
    }

    if(ProjectorExtension::isLinear())
        return projectLinear(volume);
    else // ProjectorExtension::isLinear() == false
        return projectNonLinear(volume);
}

/*!
 * Returns false, because spectral effects are non-linear (summation in intensity domain).
 */
bool SpectralEffectsExtension::isLinear() const { return false; }

// Use SerializationInterface::toVariant() documentation.
QVariant SpectralEffectsExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "SpectralEffectsExtension");

    return ret;
}

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with one key-value-pair: ("Sampling resolution", _deltaE),
 * which represents the energy resolution (in keV per bin) used for sampling of spectral effects:
 *
 * This method is used within toVariant() to serialize the object's settings.
 */
QVariant SpectralEffectsExtension::parameter() const
{
    QVariantMap ret = ProjectorExtension::parameter().toMap();

    ret.insert("Sampling resolution", _deltaE);

    return ret;
}

// Use AbstractProjector::setParameter() documentation.
void SpectralEffectsExtension::setParameter(const QVariant& parameter)
{
    ProjectorExtension::setParameter(parameter);

    auto deltaE = parameter.toMap().value("Sampling resolution", 0.0f).toFloat();
    setSpectralSamplingResolution(deltaE);
}

/*!
 * Sets the energy resolution for sampling of spectral effects to \a energyBinWidth (in keV). This
 * represents the bin width used for (spectrally) sub-sampling the projections.
 */
void SpectralEffectsExtension::setSpectralSamplingResolution(float energyBinWidth)
{
    _deltaE = energyBinWidth;

    if(_setup.isValid())
        updateSpectralInformation();
}

/*!
 * Causes an update of the spectral information to take place.
 *
 * \sa RadiationEncoder::spectralInformation().
 */
void SpectralEffectsExtension::updateSpectralInformation()
{
    _spectralInfo = RadiationEncoder::spectralInformation(_setup, _deltaE);
}

/*!
 * Returns `true` if no spectral effects need to be considered, i.e. neither the detector nor any of
 * the subvolumes in \a volume have spectral information.
 * This function throws an exception if the detector has a spectral response model and not all the
 * subvolumes in \a volume have spectral information.
 * In any other case than the aforementioned, `false` is returned.
 */
bool SpectralEffectsExtension::canBypassExtension(const CompositeVolume& volume) const
{
    const auto spectralResp = _setup.system()->detector()->hasSpectralResponseModel();
    auto allVolumesSpectral = true;
    auto noVolumeSpectral = true;

    for(auto v = 0u, nbSubVolumes = volume.nbSubVolumes(); v < nbSubVolumes; ++v)
    {
        if(volume.subVolume(v).hasSpectralInformation())
            noVolumeSpectral = false;
        else
            allVolumesSpectral = false;
    }

    if(spectralResp && !allVolumesSpectral)
        throw std::runtime_error("SpectralEffectsExtension: Cannot simulate combination of spectral "
                                 "detector response and volume data without spectral information!");

    if(!spectralResp && noVolumeSpectral) return true; // no spectral effects to be considered

    return false; // regular execution of SpectralEffectsExtension
}

/*!
 * Processes projection data in \a intensity (corresponding to the radiation energy \a energy) to
 * account for spectral detector response. For that, each intensity value is multiplied by the
 * detector response for energy \a energy.
 */
void SpectralEffectsExtension::applyDetectorResponse(ProjectionData& intensity, float energy) const
{
    if(!_setup.system()->detector()->hasSpectralResponseModel())
        return;

    // multiplicative manipulation (i.e. fraction of radiation detected)
    intensity *= _setup.system()->detector()->spectralResponseModel()->valueAt(energy);
}

/*!
 * Computes the projections from \a volume with a linear nested projector.
 *
 * The internal workflow is as follows:
 * 1. Compute forward projections of the material density of all subvolumes in \a volume.
 * 2. For each energy bin: compute intensity using singleBinIntensityLinear() and add result to
 * total sum.
 * 3. Transform final result to extinction domain.
 */
ProjectionData SpectralEffectsExtension::projectLinear(const CompositeVolume& volume)
{
    qDebug() << "linear case";

    // project all material densities
    const auto nbSubVolumes = volume.nbSubVolumes();
    std::vector<ProjectionData> materialProjs;
    materialProjs.reserve(nbSubVolumes);

    uint sv = 1;
    for(const auto& subVolume : volume.data())
    {
        emit notifier()->information("Projecting density of subvolume " + QString::number(sv++) +
                                     "/" + QString::number(nbSubVolumes) + ".");

        if(subVolume->isMuVolume()) // need transformation to densities
        {
            if(subVolume->hasSpectralInformation()) // conversion to density is possible
            {
                materialProjs.push_back(ProjectorExtension::project(*subVolume->densityVolume()));
            }
            else // without spectral information: "density = mu" (mass attenuation coeff = 1)
            {
                qWarning() << "A subvolume has no spectral information (material name: \"" +
                              subVolume->materialName() + "\"); treated as constant attenuation.";

                materialProjs.push_back(
                    ProjectorExtension::project(SpectralVolumeData::fromMuVolume(
                        *subVolume, std::make_shared<ConstantModel>(1.0f))));
            }
        }
        else // density information already stored in subvolume
        {
            materialProjs.push_back(ProjectorExtension::project(*subVolume));
        }
    }

    // process all energy bins and sum up intensities
    ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
    sumProj.allocateMemory(_setup.nbViews(), 0.0f);
    std::vector<float> massAttenuationCoeffs(nbSubVolumes);
    const auto binWidth = _spectralInfo.binWidth();

    for(auto bin = 0u, nbEnergyBins = _spectralInfo.nbEnergyBins(); bin < nbEnergyBins; ++bin)
    {
        emit notifier()->information("Processing energy bin " + QString::number(bin+1) +
                                     "/" + QString::number(nbEnergyBins) + ".");

        const auto binInfo = _spectralInfo.bin(bin);
        const auto binEnergy = binInfo.energy;

        // for each material (sub-volume): obtain mass attenuation coefficients for this bin
        std::transform(volume.data().cbegin(), volume.data().cend(), massAttenuationCoeffs.begin(),
                       [binEnergy, binWidth](const CompositeVolume::SubVolPtr& subVolume)
                       {
                           constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm
                           return subVolume->meanMassAttenuationCoeff(binEnergy, binWidth) * cm2mm;
                       });

        sumProj += singleBinIntensityLinear(materialProjs, massAttenuationCoeffs, binInfo);
    }

    sumProj.transformToExtinction(_spectralInfo.totalIntensity());

    return sumProj;
}

/*!
 * Computes the projections from \a volume with a non-linear nested projector.
 *
 * The internal workflow is as follows:
 * 1. Add dummy prepare steps to the setup (later used to adjust system for the energy bin).
 * 2. For each energy bin: compute intensity using singleBinIntensityNonLinear() and add result to
 * total sum.
 * 3. Transform final result to extinction domain.
 * 4. Remove dummy prepare steps to restore original setup.
 */
ProjectionData SpectralEffectsExtension::projectNonLinear(const CompositeVolume &volume)
{
    qDebug() << "non-linear case";

    addDummyPrepareSteps(); // dummy prepare step for source -> replaced in energy bin loop

    // process all energy bins and sum up intensities
    ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
    sumProj.allocateMemory(_setup.nbViews(), 0.0f);

    for(auto bin = 0u, nbEnergyBins = _spectralInfo.nbEnergyBins(); bin < nbEnergyBins; ++bin)
    {
        emit notifier()->information("Processing energy bin " + QString::number(bin+1) +
                                     "/" + QString::number(nbEnergyBins) + ".");

        sumProj += singleBinIntensityNonLinear(volume, _spectralInfo.bin(bin));
    }

    sumProj.transformToExtinction(_spectralInfo.totalIntensity());

    removeDummyPrepareSteps();

    return sumProj;
}

/*!
 * Computes the intensity image for a specific energy bin (as specified by \a binInfo) based on the
 * precomputed forward projections of material densities (\a materialProjs) and the specific mass
 * attenuation coefficients \a massAttenCoeffs for the particular energy bin (one value for each
 * material, i.e. materialProjs.size() == massAttenCoeffs.size()).
 *
 * The internal workflow is as follows:
 * 1. For each material: compute product of projection and corresponding attenuation coefficient
 * and sum up results
 * 2. Transform final sum to intensity domain (based on intensity data from \a binInfo).
 * 3. Apply (spectral) detector response (see applyDetectorResponse()).
 *
 * In case the total intensity of the energy bin is zero, the bin will be skipped and a zero image
 * is returned.
 */
ProjectionData
SpectralEffectsExtension::singleBinIntensityLinear(const std::vector<ProjectionData>& materialProjs,
                                                   const std::vector<float>& massAttenCoeffs,
                                                   const BinInformation& binInfo)
{
    ProjectionData binProj(_setup.system()->detector()->viewDimensions());
    binProj.allocateMemory(_setup.nbViews(), 0.0f);

    if(qFuzzyIsNull(std::accumulate(binInfo.intensities.cbegin(),binInfo.intensities.cend(), 0.0)))
    {
        qDebug() << "Skipped energy bin " << binInfo.energy << "keV";
        return binProj;
    }

    const auto nbMaterials = materialProjs.size();
    for(auto material = 0u; material < nbMaterials; ++material)
        binProj += materialProjs[material] * massAttenCoeffs[material];

    binProj.transformToIntensity(binInfo.intensities);
    applyDetectorResponse(binProj, binInfo.energy);

    return binProj;
}

/*!
 * Computes the projection intensity image of the volume data in \a volume for a specific energy
 * bin (as specified by \a binInfo).
 *
 * The internal workflow is as follows:
 * 1. For each material: compute forward projection of the attenuation data for the specific energy
 * bin (see SpectralVolumeData::muVolume()) and sum up results.
 * 2. Transform final sum to intensity domain (based on intensity data from \a binInfo).
 * 3. Apply (spectral) detector response (see applyDetectorResponse()).
 *
 * In case the total intensity of the energy bin is zero, the bin will be skipped and a zero image
 * is returned.
 */
ProjectionData SpectralEffectsExtension::singleBinIntensityNonLinear(const CompositeVolume& volume,
                                                                     const BinInformation& binInfo)
{
    ProjectionData binProj(_setup.system()->detector()->viewDimensions());
    binProj.allocateMemory(_setup.nbViews(), 0.0f);

    if(qFuzzyIsNull(std::accumulate(binInfo.adjustedFluxMods.cbegin(),
                                    binInfo.adjustedFluxMods.cend(), 0.0)))
    {
        qDebug() << "Skipped energy bin " << binInfo.energy << " keV";
        return binProj;
    }

    // replace dummy prepare step to account for bin specific flux
    replaceDummyPrepareSteps(binInfo, _spectralInfo.binWidth());

    ProjectorExtension::configure(_setup);

    // project all material densities
    std::vector<ProjectionData> materialProjs;
    for(auto subVol = 0u, nbSubVolumes = volume.nbSubVolumes(); subVol < nbSubVolumes; ++subVol)
        binProj += ProjectorExtension::project(*volume.muVolume(subVol, binInfo.energy,
                                                                _spectralInfo.binWidth()));

    binProj.transformToIntensity(binInfo.intensities);
    applyDetectorResponse(binProj, binInfo.energy);

    return binProj;
}

/*!
 * Adds dummy prepare steps (of type prepare::SourceParam) to the setup. Those are later used to
 * adjust the energy range and the flux of the system for the currently processed energy bin.
 *
 * \sa replaceDummyPrepareSteps().
 */
void SpectralEffectsExtension::addDummyPrepareSteps()
{
    for(auto view = 0u, nbViews = _setup.nbViews(); view < nbViews; ++view)
    {
        auto dummyPrepareStep = std::make_shared<prepare::SourceParam>();
        _setup.view(view).addPrepareStep(dummyPrepareStep);
    }
}

/*!
 * Removes all dummy prepare steps added by addDummyPrepareSteps(). Must not be called if
 * addDummyPrepareSteps() has not been called earlier.
 */
void SpectralEffectsExtension::removeDummyPrepareSteps()
{
    for(auto view = 0u, nbViews = _setup.nbViews(); view < nbViews; ++view)
        _setup.view(view).removeLastPrepareStep();
}

/*!
 * Replaces the dummy prepare steps (see addDummyPrepareSteps()) in the setup to adjust the energy
 * range and the flux of the system for the currently processed energy bin (as specified by
 * \a binInfo).
 */
void SpectralEffectsExtension::replaceDummyPrepareSteps(const BinInformation& binInfo, float binWidth)
{
    for(auto view = 0u, nbViews = _setup.nbViews(); view < nbViews; ++view)
    {
        auto sourcePrep = std::make_shared<prepare::SourceParam>();
        sourcePrep->setFluxModifier(binInfo.adjustedFluxMods[view]);
        sourcePrep->setEnergyRangeRestriction({ binInfo.energy - binWidth * 0.5f,
                                                binInfo.energy + binWidth * 0.5f });
        _setup.view(view).replacePrepareStep(sourcePrep);
    }
}

} // namespace CTL
