#define ENABLE_FROBENIUS_NORM
#include "geometrytest.h"
#include "acquisition/geometryencoder.h"
#include "acquisition/geometrydecoder.h"

#include "components/allcomponents.h"

#include "acquisition/ctsystem.h"
#include "acquisition/trajectories.h"

#include "io/basetypeio.h"
#include "io/den/denfileio.h"

using namespace CTL;

void GeometryTest::initTestCase()
{
    _cArmTestSystem = new CTsystem;
    _tubeTestSystem = new CTsystem;

    auto detector   = new CylindricalDetector(QSize(16, 64), QSizeF(1.0, 1.0), 40, 1.0_deg, 0.2);
    auto tubeGantry = new TubularGantry(1000.0, 550.0, 0.0, 90.0_deg, 0.0_deg);
    auto cArmGantry = new CarmGantry(1200.0, "C-arm Gantry");
    auto source     = new GenericSource();

    *_tubeTestSystem << tubeGantry << detector->clone() << source->clone();
    *_cArmTestSystem << cArmGantry << detector << source;
}

void GeometryTest::testGeometryEncoder()
{
    io::BaseTypeIO<io::DenFileIO> fileIO;
    auto loadedTubeGeo = fileIO.readFullGeometry("testData/tubeGeo.den", 40);
    auto loadedCarmGeo = fileIO.readFullGeometry("testData/cArmGeo.den", 40);

    // test tube acquisition
    AcquisitionSetup testSetupTube(*_tubeTestSystem);
    testSetupTube.setNbViews(10);
    testSetupTube.applyPreparationProtocol(protocols::HelicalTrajectory(3.6_deg, 1.0));

    auto geo = GeometryEncoder::encodeFullGeometry(testSetupTube);
    verifyPmatDiff(loadedTubeGeo, geo);

    // test c-arm wobble acquisition
    AcquisitionSetup testSetupCarm(*_cArmTestSystem);
    testSetupCarm.setNbViews(30);
    testSetupCarm.applyPreparationProtocol(protocols::WobbleTrajectory(200.0_deg, 750.0, 0.0_deg, 20.0_deg, 5.0));

    geo = GeometryEncoder::encodeFullGeometry(testSetupCarm);
    verifyPmatDiff(loadedCarmGeo, geo);
}

void GeometryTest::testDecoderEncoderConsistency()
{
    // define a random projection matrix
    auto P = CTL::ProjectionMatrix{
            0.4572,   -0.3581,    0.2922,   -0.4643,
           -0.0146,   -0.0782,    0.4595,    0.3491,
            0.3003,    0.4157,    0.1557,    0.4340 };
    P.normalize();

    auto ctSystem = CTL::GeometryDecoder::decodeSingleViewGeometry(CTL::SingleViewGeometry{ P },
                                                                   QSize(100, 100));

    auto encodedDecodedP = CTL::GeometryEncoder::encodeSingleViewGeometry(ctSystem).first();

    auto diff = (P - encodedDecodedP).norm();
    qInfo() << "diff: " << diff;
    QVERIFY(qFuzzyIsNull(diff));
}

void GeometryTest::testGeometryDecoder()
{
    GeometryDecoder decoder(QSize(16, 64), QSizeF(1.0, 1.0));

    AcquisitionSetup testSetupTube(*_tubeTestSystem);
    testSetupTube.setNbViews(10);
    testSetupTube.applyPreparationProtocol(protocols::HelicalTrajectory(3.6_deg, 1.0));

    auto geo = GeometryEncoder::encodeFullGeometry(testSetupTube);

    auto decodedSys = decoder.decodeFullGeometry(geo);

    auto reencodedGeo = GeometryEncoder::encodeFullGeometry(decodedSys);
    verifyPmatDiff(reencodedGeo, geo);

    // test c-arm wobble acquisition
    AcquisitionSetup testSetupCarm(*_cArmTestSystem);
    testSetupCarm.setNbViews(30);
    testSetupCarm.applyPreparationProtocol(
                protocols::WobbleTrajectory(200.0_deg, 750.0, 0.0_deg, 20.0_deg, 5.0));

    geo = GeometryEncoder::encodeFullGeometry(testSetupCarm);
    auto decodedSys2 = decoder.decodeFullGeometry(geo);
    reencodedGeo = GeometryEncoder::encodeFullGeometry(decodedSys2);
    verifyPmatDiff(reencodedGeo, geo);
}

void GeometryTest::verifyPmatDiff(const FullGeometry &toVerify, const FullGeometry &original)
{
    const double TOLERANCE = 1.0e-8;

    double sumDiff = 0.0;

    auto nbViews = toVerify.length();
    auto nbModules = toVerify.first().length();

    for(auto view = 0; view < nbViews; ++view)
        for(auto mod = 0; mod < nbModules; ++mod)
        {
            auto diff = toVerify.at(view).at(mod).normalized() - original.at(view).at(mod).normalized();
            sumDiff += diff.norm();
        }

    auto normalizedDiff = sumDiff / (double(nbViews)*double(nbModules));

    qInfo() << "diff: " << normalizedDiff;
    QVERIFY(normalizedDiff < TOLERANCE);
}
