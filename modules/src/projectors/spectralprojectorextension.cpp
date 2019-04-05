#include "spectralprojectorextension.h"
#include "components/abstractsource.h"
#include "components/abstractdetector.h"

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

ProjectionData SpectralProjectorExtension::project(const VolumeData& volume)
{
    // project material density
    auto proj = ProjectorExtension::project(volume);

    const auto srcPtr = _setup.system()->source();

    // get (view-dependent) spectra
    IntervalDataSeries spectrum;
    std::vector<std::vector<double>> spectra(_nbSamples);
    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        _setup.prepareView(view);
        spectrum = srcPtr->spectrum(_from, _to, _nbSamples);
        for(uint bin = 0; bin < _nbSamples; ++bin)
            spectra[bin].push_back(spectrum.value(bin));
    }


    const auto samplingPoints = spectrum.samplingPoints();
    const auto binWidth = spectrum.binWidth();

    // first bin
    auto sumProj = proj * volume.averageMassAttenuationFactor(samplingPoints[0], binWidth);
    sumProj.transformToIntensity(spectra[0]);

    for(uint bin = 1; bin < _nbSamples; ++bin)
    {
        const auto& binWeights = spectra[bin];
        if(qFuzzyIsNull(std::accumulate(binWeights.cbegin(),binWeights.cend(),0.0)))
            continue;
        auto binProj = proj * volume.averageMassAttenuationFactor(samplingPoints[bin], binWidth);
        binProj.transformToIntensity(binWeights);
        sumProj += binProj;
    }

    sumProj.transformToExtinction();

    return sumProj;
}

ProjectionData SpectralProjectorExtension::projectComposite(const CompositeVolume& volume)
{
    // project all material densities
    std::vector<ProjectionData> materialProjs;
    for(uint material = 0; material < volume.nbMaterials(); ++material)
        materialProjs.push_back(ProjectorExtension::project(volume.materialVolume(material)));

    const auto srcPtr = _setup.system()->source();

    // get (view-dependent) spectra
    IntervalDataSeries spectrum;
    std::vector<std::vector<double>> spectra(_nbSamples);
    for(uint view = 0; view < _setup.nbViews(); ++view)
    {
        _setup.prepareView(view);
        spectrum = srcPtr->spectrum(_from, _to, _nbSamples);
        for(uint bin = 0; bin < _nbSamples; ++bin)
            spectra[bin].push_back(spectrum.value(bin));
    }

    const auto samplingPoints = spectrum.samplingPoints();
    const auto binWidth = spectrum.binWidth();

    ProjectionData sumProj(_setup.system()->detector()->viewDimensions());
    sumProj.allocateMemory(_setup.nbViews(), 0.0f);

    ProjectionData binProj(_setup.system()->detector()->viewDimensions());
    binProj.allocateMemory(_setup.nbViews(), 0.0f);

    for(uint bin = 0; bin < _nbSamples; ++bin)
    {
        const auto& binWeights = spectra[bin];
        if(qFuzzyIsNull(std::accumulate(binWeights.cbegin(),binWeights.cend(),0.0)))
            continue;

        for(uint material = 0; material < volume.nbMaterials(); ++material)
            binProj += materialProjs[material] *
                    volume.materialVolume(material).averageMassAttenuationFactor(samplingPoints[bin],
                                                                                 binWidth);

        binProj.transformToIntensity(binWeights);
        sumProj += binProj;
    }

    sumProj.transformToExtinction();

    return sumProj;
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
