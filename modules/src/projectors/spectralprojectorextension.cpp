#include "spectralprojectorextension.h"
#include "components/abstractsource.h"
#include "components/abstractdetector.h"
#include "acquisition/preparesteps.h"

namespace CTL {

SpectralProjectorExtension::SpectralProjectorExtension(uint nbSamples)
    : _nbSamples(nbSamples)
{
}

void SpectralProjectorExtension::configure(const AcquisitionSetup& setup, const AbstractProjectorConfig& config)
{
    _setup = setup;
    _config.reset(config.clone());

    if(_nbSamples == 0)
        _nbSamples = _setup.system()->source()->spectrumDiscretizationHint();

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
    /*
    auto spectralInfo = spectralInformation();

    if(ProjectorExtension::isLinear()) // linear case
    {
        // project material density
        auto proj = ProjectorExtension::project(volume);

        // first bin
        auto sumProj = proj * volume.averageMassAttenuationFactor(spectralInfo.energyBins[0], spectralInfo.binWidth);
        sumProj.transformToIntensity(spectralInfo.intensities[0]);

        for(uint bin = 1; bin < _nbSamples; ++bin)
        {
            if(qFuzzyIsNull(std::accumulate(spectralInfo.intensities[bin].cbegin(),
                                            spectralInfo.intensities[bin].cend(),0.0)))
            {
                qDebug() << "SpectralProjectorExtension::project(): Skipped energy bin " << bin;
                continue;
            }

            auto binProj = proj * volume.averageMassAttenuationFactor(spectralInfo.energyBins[bin], spectralInfo.binWidth);
            binProj.transformToIntensity(spectralInfo.intensities[bin]);
            sumProj += binProj;
        }

        sumProj.transformToExtinction(spectralInfo.totalIntensities);

        return sumProj;
    }
    else
    {
        qDebug() << "non-linear case";

        ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
        sumProj.allocateMemory(_setup.nbViews(), 0.0f);

        ProjectionData binProj(_setup.system()->detector()->viewDimensions());
        binProj.allocateMemory(_setup.nbViews(), 0.0f);

        // create dummy prepare step -> replaced in energy bin loop
        for(uint view = 0; view < _setup.nbViews(); ++view)
        {
            auto dummyPrepareStep = std::make_shared<prepare::SourceParam>();
            _setup.view(view).addPrepareStep(dummyPrepareStep);
        }

        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            if(qFuzzyIsNull(std::accumulate(spectralInfo.adjustedFluxMods[bin].cbegin(),
                                            spectralInfo.adjustedFluxMods[bin].cend(),0.0)))
            {
                qDebug() << "SpectralProjectorExtension::project(): Skipped energy bin " << bin;
                continue;
            }

            // replace dummy prepare step to account for bin specific flux
            for(uint view = 0; view < _setup.nbViews(); ++view)
            {
                auto sourcePrep = std::make_shared<prepare::SourceParam>();
                sourcePrep->setFluxModifier(spectralInfo.adjustedFluxMods[bin][view]);
                _setup.view(view).replacePrepareStep(sourcePrep);
            }

            ProjectorExtension::configure(_setup, *_config);

            binProj = ProjectorExtension::project(volume.muVolume(spectralInfo.energyBins[bin], spectralInfo.binWidth));

            binProj.transformToIntensity(spectralInfo.intensities[bin]);
            sumProj += binProj;
        }

        sumProj.transformToExtinction(spectralInfo.totalIntensities);

        for(uint v = 0; v < _setup.nbViews(); ++v)
            _setup.view(v).removeLastPrepareStep();

        return sumProj;
    }
    */
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
    auto spectralInfo = spectralInformation();

    if(ProjectorExtension::isLinear())
    {
        // project all material densities
        std::vector<ProjectionData> materialProjs;
        for(uint material = 0; material < volume.nbMaterials(); ++material)
            materialProjs.push_back(ProjectorExtension::project(volume.materialVolume(material)));

        ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
        sumProj.allocateMemory(_setup.nbViews(), 0.0f);

        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            if(qFuzzyIsNull(std::accumulate(spectralInfo.intensities[bin].cbegin(),
                                            spectralInfo.intensities[bin].cend(),0.0)))
            {
                qDebug() << "Skipped energy bin " << bin;
                continue;
            }

            ProjectionData binProj(_setup.system()->detector()->viewDimensions());
            binProj.allocateMemory(_setup.nbViews(), 0.0f);

            for(uint material = 0; material < volume.nbMaterials(); ++material)
                binProj += materialProjs[material] *
                        volume.materialVolume(material).averageMassAttenuationFactor(spectralInfo.energyBins[bin],
                                                                                     spectralInfo.binWidth);

            binProj.transformToIntensity(spectralInfo.intensities[bin]);

            sumProj += binProj;
        }

        sumProj.transformToExtinction(spectralInfo.totalIntensities);

        return sumProj;
    }
    else
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

        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            if(qFuzzyIsNull(std::accumulate(spectralInfo.adjustedFluxMods[bin].cbegin(),
                                            spectralInfo.adjustedFluxMods[bin].cend(),0.0)))
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
                sourcePrep->setFluxModifier(spectralInfo.adjustedFluxMods[bin][view]);
                _setup.view(view).replacePrepareStep(sourcePrep);
            }

            ProjectorExtension::configure(_setup, *_config);

            // project all material densities
            std::vector<ProjectionData> materialProjs;
            for(uint material = 0; material < volume.nbMaterials(); ++material)
                binProj += ProjectorExtension::project(volume.muVolume(material,
                                                                       spectralInfo.energyBins[bin],
                                                                       spectralInfo.binWidth));

            binProj.transformToIntensity(spectralInfo.intensities[bin]);

            sumProj += binProj;
        } 

        sumProj.transformToExtinction(spectralInfo.totalIntensities);

        for(uint v = 0; v < _setup.nbViews(); ++v)
            _setup.view(v).removeLastPrepareStep();

        return sumProj;
    }

}

//void SpectralProjectorExtension::setSpectralRange(float from, float to)
//{
//    _from = from;
//    _to = to;
//}

void SpectralProjectorExtension::setSpectralSampling(uint nbSamples)
{
    _nbSamples = nbSamples;
}

SpectralProjectorExtension::SpectralInformation SpectralProjectorExtension::spectralInformation()
{
    SpectralInformation ret;

    // get (view-dependent) spectra
    const auto srcPtr = _setup.system()->source();

    IntervalDataSeries spectrum;
    ret.intensities.resize(_nbSamples);
    ret.adjustedFluxMods.resize(_nbSamples);
    ret.totalIntensities.resize(_setup.nbViews(), 0.0);

    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        _setup.prepareView(view);
        spectrum = srcPtr->spectrum(_nbSamples);
        auto globalFluxMod = srcPtr->fluxModifier();
        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            ret.adjustedFluxMods[bin].push_back(globalFluxMod * spectrum.value(bin));
            ret.intensities[bin].push_back(spectrum.value(bin) * spectrum.samplingPoint(bin));
            ret.totalIntensities[view] += ret.intensities[bin][view];
        }
    }

    ret.energyBins = spectrum.samplingPoints();
    ret.binWidth = spectrum.binWidth();

    return ret;
}


}
