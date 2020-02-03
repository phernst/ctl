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
    vol.addMaterialVolume(volume);

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
    if(ProjectorExtension::isLinear())
        return projectLinear(volume);
    else // ProjectorExtension::isLinear() == false
        return projectNonLinear(volume);
}

bool SpectralEffectsExtension::isLinear() const { return false; }

// Use SerializationInterface::fromVariant() documentation.
void SpectralEffectsExtension::fromVariant(const QVariant& variant)
{
    ProjectorExtension::fromVariant(variant);

    auto deltaE = variant.toMap().value("spectral sampling resolution", 0.0f).toFloat();
    setSpectralSamplingResolution(deltaE);
}

// Use SerializationInterface::toVariant() documentation.
QVariant SpectralEffectsExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "SpectralEffectsExtension");
    ret.insert("spectral sampling resolution", _deltaE);

    return ret;
}

void SpectralEffectsExtension::setSpectralSamplingResolution(float energyBinWidth)
{
    _deltaE = energyBinWidth;

    if(_setup.isValid())
        updateSpectralInformation();
}

void SpectralEffectsExtension::updateSpectralInformation()
{
    if(_deltaE < 0.0f)
        throw std::runtime_error("SpectralProjectorExtension::updateSpectralInformation():"
                                 " Requested negative energy resolution!");

    const uint nbViews = _setup.nbViews();

    determineSampling();
    _spectralInfo.reserveMemory(nbViews); // reserve memory

    // get (view-dependent) spectra
    for(uint view = 0; view < nbViews; ++view)
        extractViewSpectrum(view);
}

void SpectralEffectsExtension::determineSampling()
{
    const auto srcPtr = _setup.system()->source();
    const uint nbViews = _setup.nbViews();

    // find highest resolution and determine energy interval covering spectra of all views
    float highestResolution = std::numeric_limits<float>::max();
    EnergyRange fullCoverageInterval{ std::numeric_limits<float>::max(), 0.0f };

    for(uint view = 0; view < nbViews; ++view)
    {
        _setup.prepareView(view);
        auto viewEnergyRange = srcPtr->energyRange();
        auto viewReso = viewEnergyRange.width() / float(srcPtr->spectrumDiscretizationHint());
        highestResolution = std::min(highestResolution, viewReso);
        fullCoverageInterval.start() = std::min(fullCoverageInterval.start(), viewEnergyRange.start());
        fullCoverageInterval.end()   = std::max(fullCoverageInterval.end(), viewEnergyRange.end());
    }

    qDebug() << "highestResolution: " << highestResolution;
    qDebug() << "fullCoverageInterval: [" << fullCoverageInterval.start() << " , "
                                          << fullCoverageInterval.end() << "]";

    // energy resolution is unset --> use highest resolution found in all views
    if(_deltaE == 0.0f)
        _deltaE = std::max(highestResolution, 0.1f); // minimum (automatic) bin width: 0.1 keV

    // set required number of samples with a minimum of one sample and update coverage interval
    uint nbEnergyBins = std::max({ uint(std::ceil(fullCoverageInterval.width() / _deltaE)), 1u });
    fullCoverageInterval.end() = fullCoverageInterval.start() + nbEnergyBins * _deltaE;

    // store results
    _spectralInfo.nbSamples = nbEnergyBins;
    _spectralInfo.fullCoverage = fullCoverageInterval;
    _spectralInfo.highestResolution = highestResolution;
}

void SpectralEffectsExtension::extractViewSpectrum(uint view)
{
    RadiationEncoder radiationEnc(_setup.system());
    static const auto constModel = makeDataModel<ConstantModel>();

    _setup.prepareView(view);
    IntervalDataSeries spectrum = radiationEnc.finalSpectrum(_spectralInfo.fullCoverage, _spectralInfo.nbSamples);
    auto globalFluxMod = _setup.system()->source()->fluxModifier();
    auto spectralResponse = _setup.system()->detector()->hasSpectralResponseModel()
            ? _setup.system()->detector()->spectralResponseModel()
            : constModel.get();
    for(uint bin = 0; bin < _spectralInfo.nbSamples; ++bin)
    {
        auto E = spectrum.samplingPoint(bin);
        _spectralInfo.bins[bin].adjustedFluxMods[view] = globalFluxMod * spectrum.value(bin) * spectralResponse->valueAt(E);
        _spectralInfo.bins[bin].intensities[view] = spectrum.value(bin) * E;
        _spectralInfo.bins[bin].energy = E;
        _spectralInfo.totalIntensities[view] += (_spectralInfo.bins[bin].intensities[view] * spectralResponse->valueAt(E));
    }

    _spectralInfo.binWidth = spectrum.binWidth();
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
    const auto nbMaterials = volume.nbMaterials();
    std::vector<ProjectionData> materialProjs;
    for(uint material = 0; material < nbMaterials; ++material)
    {
        if(volume.materialVolume(material).isMuVolume()) // need transformation to densities
            materialProjs.push_back(ProjectorExtension::project(volume.materialVolume(material).densityVolume()));
        else // density information already stored in volume.materialVolume(material)
            materialProjs.push_back(ProjectorExtension::project(volume.materialVolume(material)));
    }

    // process all energy bins and sum up intensities
    ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
    sumProj.allocateMemory(_setup.nbViews(), 0.0f);

    for(uint bin = 0; bin < _spectralInfo.nbSamples; ++bin)
    {
        std::vector<float> mu(nbMaterials);
        for(uint material = 0; material < nbMaterials; ++material)
            mu[material] = volume.materialVolume(material).meanMassAttenuationCoeff(
                                     _spectralInfo.bins[bin].energy, _spectralInfo.binWidth);

        sumProj += singleBinIntensityLinear(materialProjs, mu, _spectralInfo.bins[bin]);
    }

    sumProj.transformToExtinction(_spectralInfo.totalIntensities);

    return sumProj;
}

ProjectionData SpectralEffectsExtension::projectNonLinear(const CompositeVolume &volume)
{
    qDebug() << "non-linear case";

    addDummyPrepareSteps(); // dummy prepare step for source -> replaced in energy bin loop

    // process all energy bins and sum up intensities
    ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
    sumProj.allocateMemory(_setup.nbViews(), 0.0f);

    for(uint bin = 0; bin < _spectralInfo.nbSamples; ++bin)
        sumProj += singleBinIntensityNonLinear(volume, _spectralInfo.bins[bin]);

    sumProj.transformToExtinction(_spectralInfo.totalIntensities);

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
    replaceDummyPrepareSteps(binInfo, _spectralInfo.binWidth);

    ProjectorExtension::configure(_setup);

    // project all material densities
    std::vector<ProjectionData> materialProjs;
    for(uint material = 0; material < volume.nbMaterials(); ++material)
        binProj += ProjectorExtension::project(volume.muVolume(material, binInfo.energy, _spectralInfo.binWidth));

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
        sourcePrep->setEnergyRangeRestriction({ binInfo.energy - binWidth/2.0, binInfo.energy + binWidth/2.0 });
        _setup.view(view).replacePrepareStep(sourcePrep);
    }
}

void SpectralEffectsExtension::SpectralInformation::reserveMemory(uint nbViews)
{
    BinInformation defaultBin;
    defaultBin.adjustedFluxMods = std::vector<double>(nbViews);
    defaultBin.intensities = std::vector<double>(nbViews);
    bins = std::vector<BinInformation>(nbSamples, defaultBin);
    totalIntensities = std::vector<double>(nbViews, 0.0);
}

} // namespace CTL
