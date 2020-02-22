#include "spectraleffectsextension.h"
#include "acquisition/preparesteps.h"
#include "acquisition/radiationencoder.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"
#include "models/stepfunctionmodels.h"
#include <limits>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(SpectralEffectsExtension)

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
 * \f$
 * \begin{align*}
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-m(E)\mathcal{F}_{\textrm{linear}}(\rho)\right]}\\
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-\mathcal{F}_{\textrm{non-linear}}(m(E)\cdot\rho)\right]}
 * \end{align*}
 * \f$
 */
ProjectionData SpectralEffectsExtension::project(const VolumeData& volume)
{
    CompositeVolume vol;
    vol.addSubVolume(volume);

    return projectComposite(vol);
}

/*!
 * \f$
 * \begin{align*}
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-\sum_{k}m_{k}(E)\mathcal{F}_{\textrm{linear}}(\rho_{k})\right]}\\
 * \epsilon & =\ln\frac{I_{0}}{\sum_{E}i_{0}(E)
 * \exp\left[-\sum_{k}\mathcal{F}_{\textrm{non-linear}}(m_{k}(E)\cdot\rho_{k})\right]}\\
 * \end{align*}
 * \f$
 */
ProjectionData SpectralEffectsExtension::projectComposite(const CompositeVolume& volume)
{
    if(canBypassExtension(volume))
    {
        qDebug() << "Bypassing SpectralEffectsExtension.";
        return ProjectorExtension::projectComposite(volume);
    }

    if(ProjectorExtension::isLinear())
        return projectLinear(volume);
    else // ProjectorExtension::isLinear() == false
        return projectNonLinear(volume);
}

bool SpectralEffectsExtension::isLinear() const { return false; }

// Use SerializationInterface::toVariant() documentation.
QVariant SpectralEffectsExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "SpectralEffectsExtension");

    return ret;
}

QVariant SpectralEffectsExtension::parameter() const
{
    QVariantMap ret = ProjectorExtension::parameter().toMap();

    ret.insert("Sampling resolution", _deltaE);

    return ret;
}

void SpectralEffectsExtension::setParameter(const QVariant& parameter)
{
    ProjectorExtension::setParameter(parameter);

    auto deltaE = parameter.toMap().value("Sampling resolution", 0.0f).toFloat();
    setSpectralSamplingResolution(deltaE);
}

void SpectralEffectsExtension::setSpectralSamplingResolution(float energyBinWidth)
{
    _deltaE = energyBinWidth;

    if(_setup.isValid())
        updateSpectralInformation();
}

void SpectralEffectsExtension::updateSpectralInformation()
{
    _spectralInfo = RadiationEncoder::spectralInformation(_setup, _deltaE);
}

/*!
 * Returns `true` if no spetral effects need to be considered, i.e. neither the detector nor any of
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

void SpectralEffectsExtension::applyDetectorResponse(ProjectionData& intensity, float energy) const
{
    if(!_setup.system()->detector()->hasSpectralResponseModel())
        return;

    // multiplicative manipulation (i.e. fraction of radiation detected)
    intensity *= _setup.system()->detector()->spectralResponseModel()->valueAt(energy);
}

ProjectionData SpectralEffectsExtension::projectLinear(const CompositeVolume& volume)
{
    // project all material densities
    const auto nbMaterials = volume.nbSubVolumes();
    std::vector<ProjectionData> materialProjs;
    for(uint material = 0; material < nbMaterials; ++material)
    {
        if(volume.subVolume(material).isMuVolume()) // need transformation to densities
            materialProjs.push_back(ProjectorExtension::project(volume.subVolume(material).densityVolume()));
        else // density information already stored in volume.materialVolume(material)
            materialProjs.push_back(ProjectorExtension::project(volume.subVolume(material)));
    }

    // process all energy bins and sum up intensities
    ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
    sumProj.allocateMemory(_setup.nbViews(), 0.0f);

    for(uint bin = 0; bin < _spectralInfo.nbEnergyBins(); ++bin)
    {
        std::vector<float> mu(nbMaterials);
        for(uint material = 0; material < nbMaterials; ++material)
            mu[material] = volume.subVolume(material).meanMassAttenuationCoeff(
                                     _spectralInfo.bin(bin).energy, _spectralInfo.binWidth());

        sumProj += singleBinIntensityLinear(materialProjs, mu, _spectralInfo.bin(bin));
    }

    sumProj.transformToExtinction(_spectralInfo.totalIntensity());

    return sumProj;
}

ProjectionData SpectralEffectsExtension::projectNonLinear(const CompositeVolume &volume)
{
    qDebug() << "non-linear case";

    addDummyPrepareSteps(); // dummy prepare step for source -> replaced in energy bin loop

    // process all energy bins and sum up intensities
    ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
    sumProj.allocateMemory(_setup.nbViews(), 0.0f);

    for(uint bin = 0; bin < _spectralInfo.nbEnergyBins(); ++bin)
        sumProj += singleBinIntensityNonLinear(volume, _spectralInfo.bin(bin));

    sumProj.transformToExtinction(_spectralInfo.totalIntensity());

    removeDummyPrepareSteps();

    return sumProj;
}

ProjectionData SpectralEffectsExtension::singleBinIntensityLinear(const std::vector<ProjectionData>& materialProjs,
                                                                  const std::vector<float>& mu,
                                                                  const BinInformation& binInfo)
{
    constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm

    ProjectionData binProj(_setup.system()->detector()->viewDimensions());
    binProj.allocateMemory(_setup.nbViews(), 0.0f);

    if(qFuzzyIsNull(std::accumulate(binInfo.intensities.cbegin(),binInfo.intensities.cend(), 0.0)))
    {
        qDebug() << "Skipped energy bin " << binInfo.energy << "keV";
        return binProj;
    }

    const auto nbMaterials = materialProjs.size();
    for(uint material = 0; material < nbMaterials; ++material)
        binProj += materialProjs[material] * mu[material] * cm2mm; // cm^-1 --> mm^-1

    binProj.transformToIntensity(binInfo.intensities);
    applyDetectorResponse(binProj, binInfo.energy);

    return binProj;
}

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
    for(uint material = 0; material < volume.nbSubVolumes(); ++material)
        binProj += ProjectorExtension::project(volume.muVolume(material, binInfo.energy,
                                                               _spectralInfo.binWidth()));

    binProj.transformToIntensity(binInfo.intensities);
    applyDetectorResponse(binProj, binInfo.energy);

    return binProj;
}

void SpectralEffectsExtension::addDummyPrepareSteps()
{
    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        auto dummyPrepareStep = std::make_shared<prepare::SourceParam>();
        _setup.view(view).addPrepareStep(dummyPrepareStep);
    }
}

void SpectralEffectsExtension::removeDummyPrepareSteps()
{
    for(uint v = 0; v < _setup.nbViews(); ++v)
        _setup.view(v).removeLastPrepareStep();
}

void SpectralEffectsExtension::replaceDummyPrepareSteps(const BinInformation& binInfo, float binWidth)
{
    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        auto sourcePrep = std::make_shared<prepare::SourceParam>();
        sourcePrep->setFluxModifier(binInfo.adjustedFluxMods[view]);
        sourcePrep->setEnergyRangeRestriction({ binInfo.energy - binWidth * 0.5f,
                                                binInfo.energy + binWidth * 0.5f });
        _setup.view(view).replacePrepareStep(sourcePrep);
    }
}

} // namespace CTL
