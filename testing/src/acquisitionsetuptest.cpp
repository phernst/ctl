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
    QTest::newRow("tubeCurrent")
            << (BasePtr) new CTL::protocols::TubeCurrentModulation(std::vector<double>{}) << false;
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
    CTL::protocols::WobbleTrajectory wobbleTraj(100, 3.0, 400.0);

    QVERIFY(setup.isValid());

    setup.addPreparationProtocol(helTraj);
    QVERIFY(setup.isValid());

    setup.addPreparationProtocol(wobbleTraj);
    QVERIFY(!setup.isValid());  // expect: false --> wobbleTraj not applicable to TubularGantry system
}

void AcquisitionSetupTest::testProtocolValidityChecks()
{
    QFETCH(CTL::AbstractPreparationProtocol*, protocol);
    QFETCH(bool, expected);

    QCOMPARE(protocol->isApplicableTo(_testSetup), expected);

    delete protocol;
}
