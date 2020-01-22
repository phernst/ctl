#include "spectraleffectsextension.h"
#include "acquisition/preparesteps.h"
#include "acquisition/radiationencoder.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"
#include "models/stepfunctionmodels.h"
#include <limits>

namespace CTL {

SpectralEffectsExtension::SpectralEffectsExtension(float energyBinWidth)
    : _deltaE(energyBinWidth)
{
}

void SpectralEffectsExtension::configure(const AcquisitionSetup& setup,
                                           const AbstractProjectorConfig& config)
{
    _setup = setup;
    _config.reset(config.clone());

    updateSpectralInformation();

    ProjectorExtension::configure(setup, config);
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
    {
        constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm

        // project all material densities
        std::vector<ProjectionData> materialProjs;
        for(uint material = 0; material < volume.nbMaterials(); ++material)
        {
            if(volume.materialVolume(material).isMuVolume())
                materialProjs.push_back(ProjectorExtension::project(volume.materialVolume(material).densityVolume()));
            else // density information in volume.materialVolume(material)
                materialProjs.push_back(ProjectorExtension::project(volume.materialVolume(material)));
        }

        ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
        sumProj.allocateMemory(_setup.nbViews(), 0.0f);

        for(uint bin = 0; bin < _spectralInfo.nbSamples; ++bin)
        {
            if(qFuzzyIsNull(std::accumulate(_spectralInfo.intensities[bin].cbegin(),
                                            _spectralInfo.intensities[bin].cend(), 0.0)))
            {
                qDebug() << "Skipped energy bin " << bin;
                continue;
            }

            ProjectionData binProj(_setup.system()->detector()->viewDimensions());
            binProj.allocateMemory(_setup.nbViews(), 0.0f);

            for(uint material = 0; material < volume.nbMaterials(); ++material)
                binProj += materialProjs[material]
                           * volume.materialVolume(material).meanMassAttenuationCoeff(
                                 _spectralInfo.energyBins[bin], _spectralInfo.binWidth)
                           * cm2mm; // cm^-1 --> mm^-1

            //binProj *= 0.1f; // cm^-1 --> mm^-1
            binProj.transformToIntensity(_spectralInfo.intensities[bin]);
            applyDetectorResponse(binProj, _spectralInfo.energyBins[bin]);

            sumProj += binProj;
        }

        sumProj.transformToExtinction(_spectralInfo.totalIntensities);

        return sumProj;
    }
    else // ProjectorExtension::isLinear() == false
    {
        qDebug() << "non-linear case";

        ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
        sumProj.allocateMemory(_setup.nbViews(), 0.0f);

        // create dummy prepare step -> replaced in energy bin loop
        for(uint view = 0; view < _setup.nbViews(); ++view)
        {
            auto dummyPrepareStep = std::make_shared<prepare::SourceParam>();
            _setup.view(view).addPrepareStep(dummyPrepareStep);
        }

        for(uint bin = 0; bin < _spectralInfo.nbSamples; ++bin)
        {
            if(qFuzzyIsNull(std::accumulate(_spectralInfo.adjustedFluxMods[bin].cbegin(),
                                            _spectralInfo.adjustedFluxMods[bin].cend(), 0.0)))
            {
                qDebug() << "Skipped energy bin " << bin;
                continue;
            }

            ProjectionData binProj(_setup.system()->detector()->viewDimensions());
            binProj.allocateMemory(_setup.nbViews(), 0.0f);

            // replace dummy prepare step to account for bin specific flux
            for(uint view = 0; view < _setup.nbViews(); ++view)
            {
                auto sourcePrep = std::make_shared<prepare::SourceParam>();
                sourcePrep->setFluxModifier(_spectralInfo.adjustedFluxMods[bin][view]);
                sourcePrep->setEnergyRangeRestriction({ _spectralInfo.energyBins[bin] - _spectralInfo.binWidth/2.0,
                                                        _spectralInfo.energyBins[bin] + _spectralInfo.binWidth/2.0});
                _setup.view(view).replacePrepareStep(sourcePrep);
            }

            ProjectorExtension::configure(_setup, *_config);

            // project all material densities
            std::vector<ProjectionData> materialProjs;
            for(uint material = 0; material < volume.nbMaterials(); ++material)
                binProj += ProjectorExtension::project(volume.muVolume(
                    material, _spectralInfo.energyBins[bin], _spectralInfo.binWidth));

            binProj.transformToIntensity(_spectralInfo.intensities[bin]);
            applyDetectorResponse(binProj, _spectralInfo.energyBins[bin]);

            sumProj += binProj;
        }

        sumProj.transformToExtinction(_spectralInfo.totalIntensities);

        for(uint v = 0; v < _setup.nbViews(); ++v)
            _setup.view(v).removeLastPrepareStep();

        return sumProj;
    }
}

bool SpectralEffectsExtension::isLinear() const { return false; }

void SpectralEffectsExtension::setSpectralSamplingResolution(float energyBinWidth)
{
    _deltaE = energyBinWidth;
}

void SpectralEffectsExtension::updateSpectralInformation()
{
    const auto srcPtr = _setup.system()->source();
    const uint nbViews = _setup.nbViews();

    if(_deltaE < 0.0f)
        throw std::runtime_error("SpectralProjectorExtension::updateSpectralInformation():"
                                 " Requested negative energy resolution!");

    // analyze maximum required resolution
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

    // energy resolution is unset --> use automatic determination of highest resolution
    if(_deltaE == 0.0f)
        _deltaE = std::max(highestResolution, 0.1f); // minimum (automatic) bin width: 0.1 keV

    // set required number of samples with a minimum of one sample
    uint nbEnergyBins = std::max(uint(std::ceil(fullCoverageInterval.width() / _deltaE)), 1u);
    _spectralInfo.nbSamples = nbEnergyBins;

    fullCoverageInterval.end() = fullCoverageInterval.start() + nbEnergyBins * _deltaE;

    // get (view-dependent) spectra
    IntervalDataSeries spectrum;
    _spectralInfo.intensities      = std::vector<std::vector<double>>(nbEnergyBins, std::vector<double>(nbViews));
    _spectralInfo.adjustedFluxMods = std::vector<std::vector<double>>(nbEnergyBins, std::vector<double>(nbViews));
    _spectralInfo.totalIntensities = std::vector<double>(nbViews, 0.0);

    RadiationEncoder radiationEnc(_setup.system());
    auto constModel = makeDataModel<ConstantModel>();

    for(uint view = 0; view < nbViews; ++view)
    {
        _setup.prepareView(view);
        spectrum = radiationEnc.finalSpectrum(fullCoverageInterval, nbEnergyBins);
        auto globalFluxMod = srcPtr->fluxModifier();
        auto spectralResponse = _setup.system()->detector()->hasSpectralResponseModel()
                ? _setup.system()->detector()->spectralResponseModel()
                : constModel.get();
        for(uint bin = 0; bin < nbEnergyBins; ++bin)
        {
            auto E = spectrum.samplingPoint(bin);
            _spectralInfo.adjustedFluxMods[bin][view] = globalFluxMod * spectrum.value(bin) * spectralResponse->valueAt(E);
            _spectralInfo.intensities[bin][view] = spectrum.value(bin) * E;
            _spectralInfo.totalIntensities[view] += (_spectralInfo.intensities[bin][view] * spectralResponse->valueAt(E));
        }
    }

    _spectralInfo.energyBins = spectrum.samplingPoints();
    _spectralInfo.binWidth = spectrum.binWidth();

    qDebug() << "binWidth: " << _spectralInfo.binWidth << _spectralInfo.energyBins;
}

void SpectralEffectsExtension::applyDetectorResponse(ProjectionData& intensity, float energy) const
{
    if(!_setup.system()->detector()->hasSpectralResponseModel())
        return;

    // multiplicative manipulation (i.e. fraction of radiation detected)
    intensity *= _setup.system()->detector()->spectralResponseModel()->valueAt(energy);
}

} // namespace CTL
