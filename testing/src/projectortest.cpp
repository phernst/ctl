#include "projectortest.h"
#include "acquisition/ctsystembuilder.h"
#include "acquisition/trajectories.h"
#include "acquisition/preparesteps.h"
#include "components/allcomponents.h"

#include "projectors/arealfocalspotextension.h"
#include "projectors/poissonnoiseextension.h"
#include "projectors/raycasterprojector.h"
#include "projectors/spectraleffectsextension.h"

#include "io/ctldatabase.h"

using namespace CTL;

const bool ENABLE_INTERPOLATION_IN_RAYCASTER = true;

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

void ProjectorTest::testSpectralExtension()
{
    // assemble CT system
    auto flatPanel = makeComponent<FlatPanelDetector>(QSize(50, 50), QSizeF(1.0, 1.0), "Flat panel detector");
    auto tube = makeComponent<XrayTube>(QSizeF(1.0, 1.0), 80.0, 100000.0, "X-ray tube");
    auto tubeGantry = makeComponent<CarmGantry>(1200.0);

    //tube->setSpectrumModel(new HeuristicCubicSpectrumModel());

    CTSystem system;
    system.addComponent(std::move(flatPanel));
    system.addComponent(std::move(tube));
    system.addComponent(std::move(tubeGantry));

    auto volume = SpectralVolumeData::ball(40.0f, 0.5f, 1.0f,
                                                 database::attenuationModel(database::Composite::Water));
    auto volume2 = SpectralVolumeData::ball(30.0f, 0.5f, 0.3f,
                                                  database::attenuationModel(database::Composite::Bone_Cortical));

    CompositeVolume compVol;
    compVol.addSubVolume(volume);
    compVol.addSubVolume(volume2);

    auto photonsPerPixel = 100000.0;
    auto simpleSys = SimpleCTSystem::fromCTsystem(system);
    auto fluxAdjustFactor = photonsPerPixel / simpleSys.photonsPerPixelMean();

    AcquisitionSetup setup(simpleSys);
    setup.setNbViews(5);
    setup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0));

    for(uint v = 0; v < setup.nbViews(); ++v)
    {
        auto srcPrep = std::make_shared<prepare::XrayTubeParam>();
        srcPrep->setTubeVoltage(80.0 + 20.0/setup.nbViews() * v);
        srcPrep->setEmissionCurrent(fluxAdjustFactor*(10000.0 + 10000.0/setup.nbViews() * v));
        setup.view(v).addPrepareStep(srcPrep);
    }

    // configure a projector and project volume
    OCL::RayCasterProjector* myProjector = new OCL::RayCasterProjector;
    myProjector->settings().interpolate = ENABLE_INTERPOLATION_IN_RAYCASTER;

    PoissonNoiseExtension* noiseExt = new PoissonNoiseExtension;
    noiseExt->setFixedSeed(1337u);
    noiseExt->use(myProjector);

    SpectralEffectsExtension* spectralExt = new SpectralEffectsExtension;
    spectralExt->setSpectralSamplingResolution(15.0f);
    //spectralExt->setSpectralRange(0,150);
    spectralExt->use(noiseExt);
    spectralExt->configure(setup);


    io::BaseTypeIO<io::DenFileIO> io;
    // Non-linear composite
    auto proj = spectralExt->projectComposite(compVol);
    //io.write(proj, "testData/spectral_nonlin_composite.den");
    auto groundTruth = io.readProjections("testData/spectralExtension/spectral_nonlin_composite.den");
    auto diff = proj-groundTruth;
    auto mean = projectionMean(diff);
    auto var  = projectionVariance(diff);
    qInfo() << mean << var;
    QVERIFY2(std::abs(mean) < 0.01, "Non-linear composite failed");
    QVERIFY2(var < 0.01, "Non-linear composite failed");

    // Non-linear simple
    proj = spectralExt->project(volume);
    //io.write(proj, "testData/spectral_nonlin_simple.den");
    groundTruth = io.readProjections("testData/spectralExtension/spectral_nonlin_simple.den");
    diff = proj-groundTruth;
    mean = projectionMean(diff);
    var  = projectionVariance(diff);
    qInfo() << mean << var;
    QVERIFY2(std::abs(mean) < 0.01, "Non-linear simple failed");
    QVERIFY2(var < 0.01, "Non-linear simple failed");

    spectralExt->release();
    noiseExt->release();
    delete noiseExt;

    spectralExt->use(myProjector);

    // Linear composite
    proj = spectralExt->projectComposite(compVol);
    //io.write(proj, "testData/spectral_lin_composite.den");
    groundTruth = io.readProjections("testData/spectralExtension/spectral_lin_composite.den");
    diff = proj-groundTruth;
    mean = projectionMean(diff);
    var  = projectionVariance(diff);
    qInfo() << mean << var;
    QVERIFY2(std::abs(mean) < 0.01, "Linear composite failed");
    QVERIFY2(var < 0.01, "Linear composite failed");

    // Linear simple
    proj = spectralExt->project(volume);
    //io.write(proj, "testData/spectral_lin_simple.den");
    groundTruth = io.readProjections("testData/spectralExtension/spectral_lin_simple.den");
    diff = proj-groundTruth;
    mean = projectionMean(diff);
    var  = projectionVariance(diff);
    qInfo() << mean << var;
    QVERIFY2(std::abs(mean) < 0.01, "Linear simple failed");
    QVERIFY2(var < 0.01, "Linear simple failed");

    delete spectralExt;
}

void ProjectorTest::poissonSimulation(double meanPhotons,
                                      double projAngle,
                                      uint nbRepetitions) const
{
    CTSystem theSystem;
    auto detector = new FlatPanelDetector(QSize(50, 50), QSizeF(2.0, 2.0), "Flat detector");
    auto gantry = new CarmGantry(1200.0, "Gantry");
    auto source = new XrayLaser(75.0, 1.0, "my tube");


    source->setFocalSpotSize(QSizeF(5.0, 5.0));
    theSystem << detector << gantry << source;

    auto calibPower = meanPhotons / SimpleCTSystem::fromCTsystem(theSystem).photonsPerPixelMean();
    source->setRadiationOutput(calibPower);

    AcquisitionSetup setup(theSystem);
    setup.setNbViews(1);
    setup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0, projAngle, 0.0));

    auto projector = makeProjector<OCL::RayCasterProjector>();
    projector->settings().interpolate = ENABLE_INTERPOLATION_IN_RAYCASTER;
    projector->configure(setup);

    auto projectionsClean = projector->project(_testVolume);

    // pad views with same PrepareSteps
    setup.setNbViews(nbRepetitions);
    auto firstView = setup.view(0);
    for(auto& view : setup.views())
        for(const auto& prep : firstView.prepareSteps())
            view.addPrepareStep(prep);

    auto compoundProjector = std::move(projector) |
                             makeExtension<PoissonNoiseExtension>() |
                             makeExtension<ArealFocalSpotExtension>();

    compoundProjector->setDiscretization(QSize(2, 2));
    compoundProjector->configure(setup);

    // repeatedly compute noisy projections
    auto projsWithNoise = compoundProjector->project(_testVolume);

    // ### evaluate results ###
    evaluatePoissonSimulation(projsWithNoise, projectionsClean, setup.system()->photonsPerPixelMean());
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

double ProjectorTest::projectionMean(const ProjectionData &projections) const
{
    double total = 0.0;

    for(uint view = 0; view < projections.nbViews(); ++view)
        for(uint mod = 0; mod < projections.view(view).nbModules(); ++mod)
        {
            const auto& data = projections.view(view).module(mod).constData();
            for(auto pix : data)
                total += pix;
        }

    unsigned long pixelPerView = projections.viewDimensions().nbModules
        * projections.viewDimensions().nbChannels * projections.viewDimensions().nbRows;

    return total / static_cast<double>(projections.nbViews() * pixelPerView);
}

double ProjectorTest::projectionVariance(const ProjectionData &projections) const
{
    double localMean = projectionMean(projections);

    double totalVariance = 0.0;

    for(uint view = 0; view < projections.nbViews(); ++view)
        for(uint mod = 0; mod < projections.view(view).nbModules(); ++mod)
        {
            const auto& data = projections.view(view).module(mod).constData();
            for(auto pix : data)
                totalVariance += (pix - localMean) * (pix - localMean);
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
