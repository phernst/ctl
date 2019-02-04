#ifndef ACQUISITIONSETUPTEST_H
#define ACQUISITIONSETUPTEST_H

#include <QtTest>
#include "acquisition/acquisitionsetup.h"

class AcquisitionSetupTest : public QObject
{
    Q_OBJECT

public:
    AcquisitionSetupTest();

private Q_SLOTS:
    void testSystemValidityCheck();
    void testProtocolValidityChecks();
    void testProtocolValidityChecks_data();
    void testFlyingFocalSpotProtocol();

private:
    CTL::AcquisitionSetup _testSetup;
};

#endif // ACQUISITIONSETUPTEST_H
