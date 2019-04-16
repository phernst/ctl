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

    // get (view-dependent) spectra
    const auto srcPtr = _setup.system()->source();

    IntervalDataSeries spectrum;
    std::vector<std::vector<double>> spectraN(_nbSamples);
    std::vector<std::vector<double>> spectraI(_nbSamples);
    std::vector<std::vector<double>> fluxMods(_nbSamples);
    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        _setup.prepareView(view);
        spectrum = srcPtr->spectrum(_from, _to, _nbSamples);
        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            fluxMods[bin].push_back(srcPtr->fluxModifier());
            spectraN[bin].push_back(spectrum.value(bin));
            spectraI[bin].push_back(spectrum.value(bin) * spectrum.samplingPoint(bin));
        }

        // create dummy prepare step -> replaced in energy bin loop
        auto dummyPrepareStep = std::make_shared<prepare::SourceParam>();
        _setup.view(view).addPrepareStep(dummyPrepareStep);
    }

    const auto samplingPoints = spectrum.samplingPoints();
    const auto binWidth = spectrum.binWidth();


    if(ProjectorExtension::isLinear()) // linear case
    {
        // project material density
        auto proj = ProjectorExtension::project(volume);

        // first bin
        auto sumProj = proj * volume.averageMassAttenuationFactor(samplingPoints[0], binWidth);
        sumProj.transformToIntensity(spectraI[0]);

        for(uint bin = 1; bin < _nbSamples; ++bin)
        {
            const auto& binWeights = spectraI[bin];
            if(qFuzzyIsNull(std::accumulate(binWeights.cbegin(),binWeights.cend(),0.0)))
                continue;

            auto binProj = proj * volume.averageMassAttenuationFactor(samplingPoints[bin], binWidth);
            binProj.transformToIntensity(binWeights);
            sumProj += binProj;
        }

        double intens = 0.0;
        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            qInfo() << "intens:" << spectraI[bin][0];
            qInfo() << volume.averageMassAttenuationFactor(samplingPoints[bin],binWidth);

            intens += spectraI[bin][0];
        }

        sumProj.transformToExtinction(intens);

        return sumProj;
    }
    else
    {
        qInfo() << "non-linear case";

        ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
        sumProj.allocateMemory(_setup.nbViews(), 0.0f);

        ProjectionData binProj(_setup.system()->detector()->viewDimensions());
        binProj.allocateMemory(_setup.nbViews(), 0.0f);

        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            const auto& binWeights = spectraN[bin];
            if(qFuzzyIsNull(std::accumulate(binWeights.cbegin(),binWeights.cend(),0.0)))
                continue;

            // replace dummy prepare step to account for bin specific flux
            for(uint view = 0; view < _setup.nbViews(); ++view)
            {
                auto binAdjustedFluxMod = fluxMods[bin][view] * binWeights[view];
                auto sourcePrep = std::make_shared<prepare::SourceParam>();
                sourcePrep->setFluxModifier(binAdjustedFluxMod);
                _setup.view(view).replacePrepareStep(sourcePrep);
            }

            ProjectorExtension::configure(_setup, *_config);

            binProj = ProjectorExtension::project(volume.muVolume(samplingPoints[bin], binWidth));

            binProj.transformToIntensity(spectraI[bin]);
            sumProj += binProj;
        }

        double intens = 0.0;
        for(uint bin = 0; bin < _nbSamples; ++bin)
            intens += spectraI[bin][0];

        sumProj.transformToExtinction(intens);

        return sumProj;
    }
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
    // get (view-dependent) spectra
    const auto srcPtr = _setup.system()->source();

    IntervalDataSeries spectrum;
    std::vector<std::vector<double>> relPhotonCnts(_nbSamples);
    std::vector<std::vector<double>> intensities(_nbSamples);
    std::vector<std::vector<double>> fluxMods(_nbSamples);
    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        _setup.prepareView(view);
        spectrum = srcPtr->spectrum(_from, _to, _nbSamples);
        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            fluxMods[bin].push_back(srcPtr->fluxModifier());
            relPhotonCnts[bin].push_back(spectrum.value(bin));
            intensities[bin].push_back(spectrum.value(bin) * spectrum.samplingPoint(bin));
        }

        // create dummy prepare step -> replaced in energy bin loop
        auto dummyPrepareStep = std::make_shared<prepare::SourceParam>();
        _setup.view(view).addPrepareStep(dummyPrepareStep);
    }

    const auto energyBins = spectrum.samplingPoints();
    const auto binWidth = spectrum.binWidth();

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
            if(qFuzzyIsNull(std::accumulate(intensities[bin].cbegin(),
                                            intensities[bin].cend(),0.0)))
            {
                qDebug() << "Skipped energy bin " << bin;
                continue;
            }

            ProjectionData binProj(_setup.system()->detector()->viewDimensions());
            binProj.allocateMemory(_setup.nbViews(), 0.0f);

            for(uint material = 0; material < volume.nbMaterials(); ++material)
                binProj += materialProjs[material] *
                        volume.materialVolume(material).averageMassAttenuationFactor(energyBins[bin],
                                                                                     binWidth);

            binProj.transformToIntensity(intensities[bin]);

            sumProj += binProj;
        }

        double totalIntensity = 0.0;
        for(uint bin = 0; bin < _nbSamples; ++bin)
            totalIntensity += intensities[bin][0];

        sumProj.transformToExtinction(totalIntensity);

        return sumProj;
    }
    else
    {
        qInfo() << "non-linear case";

        ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
        sumProj.allocateMemory(_setup.nbViews(), 0.0f);

        for(uint bin = 0; bin < _nbSamples; ++bin)
        {
            if(qFuzzyIsNull(std::accumulate(intensities[bin].cbegin(),
                                            intensities[bin].cend(),0.0)))
            {
                qDebug() << "Skipped energy bin " << bin;
                continue;
            }

            ProjectionData binProj(_setup.system()->detector()->viewDimensions());
            binProj.allocateMemory(_setup.nbViews(), 0.0f);

            // replace dummy prepare step to account for bin specific flux
            for(uint view = 0; view < _setup.nbViews(); ++view)
            {
                auto binAdjustedFluxMod = fluxMods[bin][view] * relPhotonCnts[bin][view];
                auto sourcePrep = std::make_shared<prepare::SourceParam>();
                sourcePrep->setFluxModifier(binAdjustedFluxMod);
                _setup.view(view).replacePrepareStep(sourcePrep);
            }

            ProjectorExtension::configure(_setup, *_config);

            // project all material densities
            std::vector<ProjectionData> materialProjs;
            for(uint material = 0; material < volume.nbMaterials(); ++material)
                binProj += ProjectorExtension::project(volume.muVolume(material,
                                                                       energyBins[bin],
                                                                       binWidth));

            binProj.transformToIntensity(intensities[bin]);

            sumProj += binProj;
        }

        double totalIntensity = 0.0;
        for(uint bin = 0; bin < _nbSamples; ++bin)
            totalIntensity += intensities[bin][0];

        sumProj.transformToExtinction(totalIntensity);

        return sumProj;
    }


}

void SpectralProjectorExtension::setSpectralRange(float from, float to)
{
    _from = from;
    _to = to;
}

void SpectralProjectorExtension::setSpectralSampling(uint nbSamples)
{
    _nbSamples = nbSamples;
}


}
