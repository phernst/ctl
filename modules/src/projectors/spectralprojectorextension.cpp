#include "spectralprojectorextension.h"
#include "acquisition/preparesteps.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"

namespace CTL {

SpectralProjectorExtension::SpectralProjectorExtension(float energyBinWidth)
    : _deltaE(energyBinWidth)
{
}

void SpectralProjectorExtension::configure(const AcquisitionSetup& setup,
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
ProjectionData SpectralProjectorExtension::project(const VolumeData& volume)
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
ProjectionData SpectralProjectorExtension::projectComposite(const CompositeVolume& volume)
{
    if(ProjectorExtension::isLinear())
    {
        // project all material densities
        std::vector<ProjectionData> materialProjs;
        for(uint material = 0; material < volume.nbMaterials(); ++material)
            materialProjs.push_back(ProjectorExtension::project(volume.materialVolume(material)));

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
                           * volume.materialVolume(material).averageMassAttenuationFactor(
                                 _spectralInfo.energyBins[bin], _spectralInfo.binWidth);

            binProj *= 0.1f; // cm^-1 --> mm^-1
            binProj.transformToIntensity(_spectralInfo.intensities[bin]);

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
                _setup.view(view).replacePrepareStep(sourcePrep);
            }

            ProjectorExtension::configure(_setup, *_config);

            // project all material densities
            std::vector<ProjectionData> materialProjs;
            for(uint material = 0; material < volume.nbMaterials(); ++material)
                binProj += ProjectorExtension::project(volume.muVolume(
                    material, _spectralInfo.energyBins[bin], _spectralInfo.binWidth));

            binProj.transformToIntensity(_spectralInfo.intensities[bin]);

            sumProj += binProj;
        }

        sumProj.transformToExtinction(_spectralInfo.totalIntensities);

        for(uint v = 0; v < _setup.nbViews(); ++v)
            _setup.view(v).removeLastPrepareStep();

        return sumProj;
    }
}

bool SpectralProjectorExtension::isLinear() const { return false; }

void SpectralProjectorExtension::setSpectralSamplingResolution(float energyBinWidth)
{
    _deltaE = energyBinWidth;
}

void SpectralProjectorExtension::updateSpectralInformation()
{
    const auto srcPtr = _setup.system()->source();
    const uint nbViews = _setup.nbViews();

    if(_deltaE < 0.0f)
        throw std::runtime_error("SpectralProjectorExtension::updateSpectralInformation():"
                                 " Requested negative energy resolution!");

    // analyze maximum required resolution
    float highestResolution = MAXFLOAT;
    AbstractSource::EnergyRange fullCoverageInterval{ MAXFLOAT, 0.0f };

    for(uint view = 0; view < nbViews; ++view)
    {
        _setup.prepareView(view);
        auto viewEnergyRange = srcPtr->energyRange();
        auto viewReso = viewEnergyRange.width() / float(srcPtr->spectrumDiscretizationHint());
        highestResolution = std::min(highestResolution, viewReso);
        fullCoverageInterval.from = std::min(fullCoverageInterval.from, viewEnergyRange.from);
        fullCoverageInterval.to   = std::max(fullCoverageInterval.to, viewEnergyRange.to);
    }

    qDebug() << "highestResolution: " << highestResolution;
    qDebug() << "fullCoverageInterval: [" << fullCoverageInterval.from << " , "
                                          << fullCoverageInterval.to << "]";

    // energy resolution is unset --> use automatic determination of highest resolution
    if(_deltaE == 0.0f)
        _deltaE = std::max(highestResolution, 0.1f); // minimum (automatic) bin width: 0.1 keV

    // set required number of samples with a minimum of one sample
    uint nbSamples = std::max(uint(std::ceil(fullCoverageInterval.width() / _deltaE)), 1u);
    _spectralInfo.nbSamples = nbSamples;

    fullCoverageInterval.to = fullCoverageInterval.from + nbSamples * _deltaE;

    // get (view-dependent) spectra
    IntervalDataSeries spectrum;
    _spectralInfo.intensities.resize(nbSamples);
    _spectralInfo.adjustedFluxMods.resize(nbSamples);
    _spectralInfo.totalIntensities.resize(_setup.nbViews(), 0.0);

    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        _setup.prepareView(view);
        spectrum = srcPtr->spectrum(fullCoverageInterval, nbSamples);
        auto globalFluxMod = srcPtr->fluxModifier();
        for(uint bin = 0; bin < nbSamples; ++bin)
        {
            _spectralInfo.adjustedFluxMods[bin].push_back(globalFluxMod * spectrum.value(bin));
            _spectralInfo.intensities[bin].push_back(spectrum.value(bin)
                                                     * spectrum.samplingPoint(bin));
            _spectralInfo.totalIntensities[view] += _spectralInfo.intensities[bin][view];
        }
    }

    _spectralInfo.energyBins = spectrum.samplingPoints();
    _spectralInfo.binWidth = spectrum.binWidth();

    qDebug() << "binWidth: " << _spectralInfo.binWidth;
}

} // namespace CTL
