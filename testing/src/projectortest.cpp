#include "projectortest.h"
#include "acquisition/ctsystembuilder.h"
#include "acquisition/trajectories.h"

#include "components/allcomponents.h"

#include "projectors/arealfocalspotextension.h"
#include "projectors/poissonnoiseextension.h"
#include "projectors/raycasterprojector.h"

using namespace CTL;

void ProjectorTest::initTestCase()
{
    // Volume
    VoxelVolume<float> volume(70, 70, 70);
    volume.setVoxelSize(1.0f, 1.0f, 1.0f);
    volume.fill(0.03f);
    _testVolume = std::move(volume);
}

void ProjectorTest::testPoissonExtension()
{
    poissonSimulation(10, 0.1, 100);
    poissonSimulation(1000, 0.1, 100);
    poissonSimulation(100000, 0.1, 100);

    poissonSimulation(1000, 0.4, 100);
    poissonSimulation(1000, 1.1, 100);

    poissonSimulation(10, 0.1, 200);
    poissonSimulation(1000, 0.1, 200);
    poissonSimulation(100000, 0.1, 200);
}

void ProjectorTest::poissonSimulation(double meanPhotons,
                                      double projAngle,
                                      uint nbRepetitions) const
{
    CTsystem theSystem;
    auto detector = new FlatPanelDetector(QSize(50, 50), QSizeF(2.0, 2.0), "Flat detector");
    auto gantry = new CarmGantry(1200.0, "Gantry");
    auto source = new XrayLaser(75.0, meanPhotons, "my tube");

    source->setFocalSpotSize(QSizeF(5.0, 5.0));
    theSystem << detector << gantry << source;

    AcquisitionSetup setup(theSystem);
    setup.setNbViews(1);
    setup.applyPreparationProtocol(protocols::WobbleTrajectory(0.0, 750.0, projAngle, 0.0));

    auto rcConfig = OCL::RayCasterProjector::Config();
    // rcConfig.interpolate = false;
    auto projector = makeProjector<OCL::RayCasterProjector>();
    projector->configure(setup, rcConfig);

    auto projectionsClean = projector->project(_testVolume);

    // pad views with same PrepareSteps
    setup.setNbViews(nbRepetitions);
    auto firstView = setup.view(0);
    for(auto& view : setup.views())
        view.prepareSteps = firstView.prepareSteps;

    auto poissonExt = makeExtension<PoissonNoiseExtension>(std::move(projector));
    poissonExt->setParallelizationEnabled(true);
    // poissonExt->configurate(setup, rcConfig);

    auto focalSpotExt = makeExtension<ArealFocalSpotExtension>(std::move(poissonExt));
    focalSpotExt->setDiscretization(QSize(2, 2));
    focalSpotExt->configure(setup, rcConfig);

    // repeatedly compute noisy projections
    auto projsWithNoise = focalSpotExt->project(_testVolume);

    // ### evaluate results ###
    auto intensity = setup.system()->source()->photonFlux();
    evaluatePoissonSimulation(projsWithNoise, projectionsClean, intensity);
}

void ProjectorTest::evaluatePoissonSimulation(const ProjectionData& repeatedProjs,
                                              const ProjectionData& cleanProjections,
                                              double intensity) const
{
    // allowed relative differences w.r.t. mean photon count
    constexpr double requestedPrecisionMeans = 0.01;
    // allowed relative differences w.r.t. mean photon count
    constexpr double requestedPrecisionDiff = 0.05;

    const auto nbRepetitions = repeatedProjs.nbViews();

    // mean photon count (across repetitions) and corresponding variance
    auto repMean = repetitionMean(repeatedProjs, intensity);
    auto repVar = repetitionVariance(repeatedProjs, intensity);

    // difference between mean and variance for each pixel
    // these should be equal in Poisson distributed values
    Chunk2D<double> diffMeanAndVariance = repMean - repVar;

    // average difference between mean and variance across all detector pixels
    double meanDiff = chunkMean(diffMeanAndVariance);

    // variance of differences between mean and variance across all detector pixels
    double diffVar = 0.0;
    const auto& data = diffMeanAndVariance.constData();
    for(auto pix : data)
        diffVar += pow((pix - meanDiff), 2.0);
    diffVar /= diffMeanAndVariance.nbElements();

    // average photon count across all detector pixels
    auto meanPhotonsClean
        = chunkMean(transformedToCounts(cleanProjections.view(0).module(0), intensity));
    auto meanPhotonsNoisy = chunkMean(repMean);

    auto differenceStd = sqrt(diffVar);

    auto precMeanPhotons = fabs(meanPhotonsClean - meanPhotonsNoisy) / meanPhotonsClean;
    auto precDiffMeanVariance = fabs(meanDiff) / meanPhotonsClean;

    qInfo().noquote() << "Mean number photons (original | noisy): "
                      << meanPhotonsClean << " | " << meanPhotonsNoisy
                      << " (prec.: " + QString::number(precMeanPhotons) + ")";
    qInfo().noquote() << "Difference mean-variance (" + QString::number(nbRepetitions)
                         + " repetitions): "
                      << meanDiff << " (std: " << differenceStd << ")"
                      << " (prec.: " + QString::number(precDiffMeanVariance) + ")";

    QVERIFY(precMeanPhotons < requestedPrecisionMeans);
    QVERIFY(precDiffMeanVariance < requestedPrecisionDiff);
    QVERIFY(differenceStd < meanPhotonsClean);
}

double ProjectorTest::countsMean(const ProjectionData& projections, double i_0) const
{
    double totalCounts = 0.0;

    for(uint view = 0; view < projections.nbViews(); ++view)
        for(uint mod = 0; mod < projections.view(view).nbModules(); ++mod)
        {
            const auto& data = projections.view(view).module(mod).constData();
            for(auto pix : data)
                totalCounts += i_0 * double(std::exp(-pix));
        }

    unsigned long pixelPerView = projections.viewDimensions().nbModules
        * projections.viewDimensions().nbChannels * projections.viewDimensions().nbRows;

    return totalCounts / static_cast<double>(projections.nbViews() * pixelPerView);
}

double ProjectorTest::countsVariance(const ProjectionData& projections, double i_0) const
{
    double localMean = countsMean(projections, i_0);

    double totalVariance = 0.0;

    for(uint view = 0; view < projections.nbViews(); ++view)
        for(uint mod = 0; mod < projections.view(view).nbModules(); ++mod)
        {
            const auto& data = projections.view(view).module(mod).constData();
            for(auto pix : data)
            {
                auto tmpCount = i_0 * double(std::exp(-pix));
                totalVariance += (tmpCount - localMean) * (tmpCount - localMean);
            }
        }

    unsigned long pixelPerView = projections.viewDimensions().nbModules
        * projections.viewDimensions().nbChannels * projections.viewDimensions().nbRows;

    return totalVariance / static_cast<double>(projections.nbViews() * pixelPerView);
}

Chunk2D<double> ProjectorTest::repetitionMean(const ProjectionData& repeatedProjs, double i_0) const
{
    Chunk2D<double> ret(repeatedProjs.dimensions().nbChannels, repeatedProjs.dimensions().nbRows);
    ret.allocateMemory();

    Chunk2D<double> tmp(repeatedProjs.dimensions().nbChannels, repeatedProjs.dimensions().nbRows);
    tmp.setData(std::vector<double>(tmp.nbElements(), 0));

    for(uint rep = 0; rep < repeatedProjs.nbViews(); ++rep)
    {
        tmp += transformedToCounts(repeatedProjs.view(rep).module(0), i_0);
    }

    auto rawRet = ret.rawData();
    auto rawTmp = tmp.rawData();
    for(uint i = 0; i < tmp.nbElements(); ++i)
        rawRet[i] = double(rawTmp[i]) / double(repeatedProjs.nbViews());

    return ret;
}

Chunk2D<double> ProjectorTest::repetitionVariance(const ProjectionData& repeatedProjs,
                                                  double i_0) const
{
    auto mean = repetitionMean(repeatedProjs, i_0);

    Chunk2D<double> tmp(repeatedProjs.dimensions().nbChannels, repeatedProjs.dimensions().nbRows);
    tmp.setData(std::vector<double>(tmp.nbElements(), 0.0));

    auto rawMean = mean.rawData();
    auto rawTmp = tmp.rawData();

    for(uint rep = 0; rep < repeatedProjs.nbViews(); ++rep)
    {
        auto tmpCounts = transformedToCounts(repeatedProjs.view(rep).module(0), i_0);
        auto rawTmpCounts = tmpCounts.rawData();
        for(uint i = 0; i < tmp.nbElements(); ++i)
            rawTmp[i] += pow(rawTmpCounts[i] - rawMean[i], 2.0);
    }

    return tmp / double(repeatedProjs.nbViews());
}

Chunk2D<double> ProjectorTest::transformedToCounts(const SingleViewData::ModuleData& module,
                                                   double i_0) const
{
    Chunk2D<double> ret(module.width(), module.height());
    ret.allocateMemory();

    auto rawDataPtr = ret.rawData();
    const auto& moduleDat = module.constData();
    for(uint pix = 0; pix < module.nbElements(); ++pix)
        rawDataPtr[pix] = i_0 * double(std::exp(-moduleDat[pix]));

    return ret;
}

VolumeData::Dimensions ProjectorTest::toVoxelVolumeDimensions(const io::den::Header& header)
{
    CTL::VolumeData::Dimensions ret;
    ret.x = static_cast<uint>(header.cols);
    ret.y = static_cast<uint>(header.rows);
    ret.z = static_cast<uint>(header.count);
    return ret;
}

template <typename T>
double ProjectorTest::chunkMean(const Chunk2D<T>& chunk) const
{
    double ret = 0.0;
    for(const auto& val : chunk.constData())
        ret += val;

    return ret / double(chunk.nbElements());
}
