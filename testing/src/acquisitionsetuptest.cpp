#include "acquisitionsetuptest.h"
#include "components/allcomponents.h"
#include "acquisition/acquisitionsetup.h"
#include "acquisition/preparationprotocols.h"
#include "acquisition/trajectories.h"

Q_DECLARE_METATYPE(CTL::AbstractPreparationProtocol*)

void AcquisitionSetupTest::testProtocolValidityChecks_data()
{
    typedef CTL::AbstractPreparationProtocol* BasePtr;

    QTest::addColumn<CTL::AbstractPreparationProtocol*>("protocol");
    QTest::addColumn<bool>("expected");

    /*  _testSetup
     *  system(): CTL::SimpleSystem(CTL::FlatPanelDetector(QSize(100,100),QSizeF(1.0,1.0)),
     *                              CTL::TubularGantry(1000.0, 600.0),
     *                              CTL::XrayLaser()
     *  nbViews(): 10
     */

    // trajectories
    QTest::newRow("helical")
            << (BasePtr) new CTL::protocols::HelicalTrajectory(10.0) << true;
    QTest::newRow("wobble")
            << (BasePtr) new CTL::protocols::WobbleTrajectory(100, 3.0, 400.0) << false;
    QTest::newRow("circLine")
            << (BasePtr) new CTL::protocols::CirclePlusLineTrajectory(100, 100, 3.0, 400, 100.0) << false;
    QTest::newRow("shortScan")
            << (BasePtr) new CTL::protocols::ShortScanTrajectory(400.0) << false;

    // other protocols
    QTest::newRow("ffsProt")
            << (BasePtr) new CTL::protocols::FlyingFocalSpot{std::vector<CTL::Vector3x1>(10)} << true;
    QTest::newRow("ffsProtFail")
            << (BasePtr) new CTL::protocols::FlyingFocalSpot(std::vector<CTL::Vector3x1>{}) << false;
    QTest::newRow("ffsProtAlternating")
            << (BasePtr) new CTL::protocols::FlyingFocalSpot({ { {1.0, 0.0, 0.0} },
                                                               { {0.0, 0.0, 1.0} } }, true) << true;
    QTest::newRow("ffsProtFail2")
            << (BasePtr) new CTL::protocols::FlyingFocalSpot({ { {1.0, 0.0, 0.0} },
                                                               { {0.0, 0.0, 1.0} } }, false) << false;
    QTest::newRow("tubeCurrent")
            << (BasePtr) new CTL::protocols::TubeCurrentModulation(std::vector<double>{}) << false;
}

void AcquisitionSetupTest::testFlyingFocalSpotProtocol()
{
    CTL::AcquisitionSetup setup(_testSetup);

    const auto pos1 = CTL::Vector3x1( {1.0, 0.0, 0.0} );
    const auto pos2 = CTL::Vector3x1( {0.0, 0.0, 1.0} );
    const auto pos3 = CTL::Vector3x1( {0.0,  1.0, 1.0} );
    const auto pos4 = CTL::Vector3x1( {1.0, -1.0, 0.0} );

    setup.applyPreparationProtocol(CTL::protocols::FlyingFocalSpot::twoAlternatingSpots(pos1, pos2));
    setup.prepareView(0);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos1);
    setup.prepareView(1);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos2);
    setup.prepareView(7);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos2);

    setup.clearViews();
    setup.applyPreparationProtocol(CTL::protocols::FlyingFocalSpot::fourAlternatingSpots(pos1, pos2, pos3, pos4));
    setup.prepareView(0);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos1);
    setup.prepareView(5);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos2);
    setup.prepareView(7);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos4);

    setup.clearViews();
    setup.applyPreparationProtocol(CTL::protocols::FlyingFocalSpot({pos1, pos2, pos3}, true));
    setup.prepareView(0);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos1);
    setup.prepareView(5);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos3);
    setup.prepareView(7);
    QVERIFY(setup.system()->source()->focalSpotPosition() == pos2);
}

AcquisitionSetupTest::AcquisitionSetupTest()
    : _testSetup(CTL::SimpleCTsystem(CTL::FlatPanelDetector(QSize(100,100),QSizeF(1.0,1.0)),
                                     CTL::TubularGantry(1000.0, 600.0),
                                     CTL::XrayLaser()))
{
    _testSetup.setNbViews(10);
}

void AcquisitionSetupTest::testSystemValidityCheck()
{
    CTL::AcquisitionSetup setup(_testSetup);

    CTL::protocols::HelicalTrajectory helTraj(10.0);
    CTL::protocols::WobbleTrajectory wobbleTraj(3.0, 400.0);

    QVERIFY(setup.isValid());

    setup.applyPreparationProtocol(helTraj);
    QVERIFY(setup.isValid());

    setup.applyPreparationProtocol(wobbleTraj);
    QVERIFY(!setup.isValid());  // expect: false --> wobbleTraj not applicable to TubularGantry system
}

void AcquisitionSetupTest::testProtocolValidityChecks()
{
    QFETCH(CTL::AbstractPreparationProtocol*, protocol);
    QFETCH(bool, expected);

    QCOMPARE(protocol->isApplicableTo(_testSetup), expected);

    delete protocol;
}
