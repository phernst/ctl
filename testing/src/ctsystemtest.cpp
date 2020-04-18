#include "acquisition/ctsystem.h"
#include "components/cylindricaldetector.h"
#include "components/tubulargantry.h"
#include "components/xraytube.h"
#include "components/xraylaser.h"
#include "components/genericbeammodifier.h"
#include "acquisition/systemblueprints.h"

#include "ctsystemtest.h"

#include "mat/mat.h"

using namespace CTL;

void CTSystemTest::initTestCase()
{
    _theTestSystem = new CTSystem;

    AbstractDetector* detector     = new CylindricalDetector(QSize(16, 64), QSizeF(1.0, 1.0), 40, 1.0_deg, 0.2);
    AbstractGantry* gantry         = new TubularGantry(1000.0, 550.0, 0.0, 90.0_deg, 0.0_deg);
    AbstractSource* source         = new XrayTube(120.0, 100);
    AbstractBeamModifier* modifier = new GenericBeamModifier("mod");

    *_theTestSystem << detector << gantry << source << modifier;
}

void CTSystemTest::cleanupTestCase()
{
    delete _theTestSystem;
}

void CTSystemTest::componentCount()
{
    QCOMPARE(_theTestSystem->nbComponents(), uint(4));
}

void CTSystemTest::validSystem()
{
    QVERIFY(_theTestSystem->isValid());
}

void CTSystemTest::simpleSystem()
{
    QVERIFY(_theTestSystem->isSimple());

    // add another detector --> should no longer be simple
    _theTestSystem->addComponent(new GenericDetector({ 5, 5 }, 1u));
    QVERIFY(!_theTestSystem->isSimple());
}

void CTSystemTest::renameCheck()
{
    // rename the system --> should no longer have default name
    _theTestSystem->rename("mySystem");
    QVERIFY(_theTestSystem->name() != CTSystem::defaultName());
}

void CTSystemTest::testSystemBuilder()
{
    auto testSystem = CTSystemBuilder::createFromBlueprint(blueprints::GenericTubularCT());

    QVERIFY(testSystem.isValid());
    QVERIFY(testSystem.isSimple());
    QCOMPARE(testSystem.name(), QStringLiteral("Tubular CT system"));
}
