#include <QCoreApplication>

#include "projectionmatrixtest.h"
#include "ctsystemtest.h"
#include "datatypetest.h"
#include "geometrytest.h"
#include "denfileiotest.h"
#include "nrrdfileiotest.h"
#include "projectortest.h"
#include "spectrumtest.h"
#include "acquisitionsetuptest.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    ProjectionMatrixTest pMat;
    CTsystemTest ctSys;
    DataTypeTest dataTest;
    GeometryTest geoTest;
    DenFileIOtest denFileTest;
    NrrdFileIOtest nrrdFileTest;
    ProjectorTest projectorTest;
    SpectrumTest spectrumTest;
    AcquisitionSetupTest acqSetupTest;

    int failedTests = 0;
    failedTests += QTest::qExec(&pMat, argc, argv);
    failedTests += QTest::qExec(&ctSys, argc, argv);
    failedTests += QTest::qExec(&dataTest, argc, argv);
    failedTests += QTest::qExec(&geoTest, argc, argv);
    failedTests += QTest::qExec(&denFileTest, argc, argv);
    failedTests += QTest::qExec(&nrrdFileTest, argc, argv);
    failedTests += QTest::qExec(&projectorTest, argc, argv);
    failedTests += QTest::qExec(&spectrumTest, argc, argv);
    failedTests += QTest::qExec(&acqSetupTest, argc, argv);

    if(failedTests)
        std::cout << "\n ##### Total number of failed tests: " << failedTests << " #####" << std::endl;
    else
        std::cout << "\n ##### All tests passed. #####" << std::endl;

    return failedTests;
}
