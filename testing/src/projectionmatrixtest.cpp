#include "projectionmatrixtest.h"
#include "mat/mat.h"
#include "img/voxelvolume.h"

using namespace CTL::mat;

// clang-format off
static const ProjectionMatrix Pinit =
ProjectionMatrix::compose({ 1000.0,    0.0, 300.0,
                               0.0, 1000.0, 250.0,
                               0.0,    0.0,   1.0 },
                          rotationMatrix(-90.0_deg, Qt::XAxis),
                          { 0.0, 300.0, 0.0 });
// clang-format on

// print infos at beginning
void ProjectionMatrixTest::initTestCase()
{
    /*
    init();
    qDebug().noquote() << "use projection matrix:\n"
                       << QString::fromStdString(P.info());
    */

    // using integers for element-wise initialization
    ProjectionMatrix tempTestInit(1, 1, 2, 3,
                                  5, 8, 13, 21,
                                  34, 55, 89, 144);
}

// do some scaling of the original PMat before each test
void ProjectionMatrixTest::init() { P = -Pinit / 300.0; }

void ProjectionMatrixTest::principalRay()
{
    auto princRay_normal = P.principalRayDirection();
    auto princRay_indirect1 = P.directionSourceToPixel(P.principalPoint());
    princRay_indirect1 /= princRay_indirect1.norm();
    auto princRay_indirect2
        = P.directionSourceToPixel(P.principalPoint(), ProjectionMatrix::NormalizeAsUnitVector);

    auto normalizationDiff = princRay_indirect1 - princRay_indirect2;
    QVERIFY(qFuzzyIsNull(normalizationDiff.norm()));

    auto principalRayDiff = princRay_normal - princRay_indirect1;
    QVERIFY(qFuzzyIsNull(principalRayDiff.norm()));
}

void ProjectionMatrixTest::sourcePosition()
{
    QVERIFY(qFuzzyIsNull((Matrix<3, 1>(0.0, 300.0, 0.0) - P.sourcePosition()).norm()));
}

void ProjectionMatrixTest::resampleDetector()
{
    P.changeDetectorResolution(2.0);
    QCOMPARE(P.principalPoint().get<0>(), 600.0);
    QCOMPARE(P.focalLength().get<1>(), 2000.0);
}

void ProjectionMatrixTest::shiftOrigin()
{
    P.shiftDetectorOrigin(8.0, 40.0);
    QCOMPARE(P.principalPoint().get<0>(), 292.0);
    QCOMPARE(P.principalPoint().get<1>(), 210.0);
}

void ProjectionMatrixTest::skewCoefficient()
{
    auto homoMat = ProjectionMatrix::compose( { 1000.0,    0.5, 300.0,
                                                   0.0, 1000.0, 250.0,
                                                   0.0,    0.0,   1.0 },
                                             Matrix<3,1>(0.0));
    QCOMPARE(homoMat.skewCoefficient(), 0.5);
    QCOMPARE(P.skewCoefficient(), 0.0);
}

void ProjectionMatrixTest::projectionOntoDetector()
{
    Matrix<3, 1> testVec(1.0, 4.0, 8.0);
    auto res1 = P * vertcat(testVec, Matrix<1, 1>(1.0));
    res1 /= res1(2);

    auto res2 = P.projectOntoDetector(testVec);

    QCOMPARE(res2(0), res1(0));
    QCOMPARE(res2(1), res1(1));
}

void ProjectionMatrixTest::equalityTest()
{
    // clang-format off
    Matrix<2, 3> matA( 1.0, 2.0, 3.0,
                       1.0, 2.0, 3.0 );
    Matrix<2, 3> minusMatA( -1.0, -2.0, -3.0,
                            -1.0, -2.0, -3.0 );
    Matrix<2, 3> matB( 1.0, 2.0, 3.0,
                       1.0, 2.0, 3.0 );
    Matrix<2, 3> matC( 1.0, 2.0, 3.0,
                       1.0, 2.01, 3.0 );
    Matrix<2, 3> minusMatC = -matC;
    // clang-format on

    QVERIFY(matA == matB);
    QVERIFY(matA != matC);
    QVERIFY(matA != minusMatA);
    QVERIFY(matA == -minusMatA);
    QVERIFY(matC == -minusMatC);
    QVERIFY(P != 1.5 * P);
    QVERIFY(P == -(-P));
}

void ProjectionMatrixTest::comparatorTest()
{
    auto P1 = P.normalized();
    auto P2 = P * 10000.0;
    PMatComparator compare;
    QVERIFY(qFuzzyIsNull(compare(P1, P2).maxError));

    P1.shiftDetectorOrigin(0.1, 2.0);
    auto result = compare(P1, P2);
    QCOMPARE(result.meanError, result.maxError);
    QCOMPARE(result.minError, result.maxError);
    QVERIFY(result.maxError >= 2.0);
    P1.changeDetectorResolution(2.0);
    QVERIFY(compare(P1, P2).minError != compare(P1, P2).maxError);

    compare.setAccuracy(2.0);
    QCOMPARE(compare.volumeGridSpacing()(0), 4.0);

    compare.setAccuracy(1.0);
    auto P3 = P;
    auto P4 = P;
    P4.shiftDetectorOrigin(42.0, 0.0);
    QCOMPARE(compare(P3, P4).meanError, 42.0);

    CTL::VoxelVolume<float> vol(12,24,48, 0.5,0.5,0.25);
    vol.setVolumeOffset(0.0,0.0,-20.0);
    compare.setNumberDetectorPixels(0, 0);
    compare.setRestrictionToDetectorArea(false);
    compare.setVolumeDefFrom(vol);
    QCOMPARE(compare.totalVolumeSize().get<0>(), 6.0);
    QCOMPARE(compare.totalVolumeSize().get<1>(), 12.0);
    QCOMPARE(compare.totalVolumeSize().get<2>(), 12.0);
    QCOMPARE(compare.volumeGridSpacing().get<0>(), 0.5);
    QCOMPARE(compare.volumeGridSpacing().get<1>(), 0.5);
    QCOMPARE(compare.volumeGridSpacing().get<2>(), 0.25);
    QCOMPARE(compare.volumeOffset().get<0>(), 0.0);
    QCOMPARE(compare.volumeOffset().get<1>(), 0.0);
    QCOMPARE(compare.volumeOffset().get<2>(), -20.0);
    QCOMPARE(compare(P3, P4).meanError, 42.0);

    compare.setVolumeGridSpacing(1.0,2.0,3.0);
    QCOMPARE(compare.totalVolumeSize().get<0>(), 6.0);
    QCOMPARE(compare.totalVolumeSize().get<1>(), 12.0);
    QCOMPARE(compare.totalVolumeSize().get<2>(), 12.0);
    QCOMPARE(compare.volumeGridSpacing().get<0>(), 1.0);
    QCOMPARE(compare.volumeGridSpacing().get<1>(), 2.0);
    QCOMPARE(compare.volumeGridSpacing().get<2>(), 3.0);

    compare.setTotalVolumeSize(12.0,12.0,24.0);
    QCOMPARE(compare.totalVolumeSize().get<0>(), 12.0);
    QCOMPARE(compare.totalVolumeSize().get<1>(), 12.0);
    QCOMPARE(compare.totalVolumeSize().get<2>(), 24.0);
    QCOMPARE(compare.volumeGridSpacing().get<0>(), 2.0);
    QCOMPARE(compare.volumeGridSpacing().get<1>(), 2.0);
    QCOMPARE(compare.volumeGridSpacing().get<2>(), 6.0);
    QCOMPARE(compare.volumeOffset().get<0>(), 0.0);
    QCOMPARE(compare.volumeOffset().get<1>(), 0.0);
    QCOMPARE(compare.volumeOffset().get<2>(), -20.0);
    QCOMPARE(compare(P3, P4).meanError, 42.0);
}

void ProjectionMatrixTest::subMatExtraction()
{
    ProjectionMatrix Pmat{ 1, 2, 3, 4,
                           5, 6, 7, 8,
                           9, 10,11,12 };

    const auto M = Pmat.subMat<0,2, 0,2>();
    const auto resM = M - Matrix<3, 3>{ 1, 2, 3,
                                        5, 6, 7,
                                        9, 10,11 };
    QVERIFY(qFuzzyIsNull(resM.norm()));

    const auto m3 = Pmat.subMat<2,2, 0,2>();
    QCOMPARE(m3.get<0>(), 9.0);
    QCOMPARE(m3.get<1>(), 10.0);
    QCOMPARE(m3.get<2>(), 11.0);

    const auto p4 = Pmat.subMat<0,2, 3,3>();
    QCOMPARE(p4.get<0>(), 4.0);
    QCOMPARE(p4.get<1>(), 8.0);
    QCOMPARE(p4.get<2>(), 12.0);

    const auto p4Rev = p4.subMat<2,0>();
    const auto p4Rev2 = Matrix<3, 1>{12, 8, 4};
    QCOMPARE(p4Rev2, p4Rev);

    const auto flipud = Pmat.subMat<2,0, 0,3>();
    const auto resud = flipud - ProjectionMatrix{ 9, 10,11,12,
                                                  5, 6, 7, 8,
                                                  1, 2, 3, 4};
    QVERIFY(qFuzzyIsNull(resud.norm()));

    const auto fliprl = Pmat.subMat<0,2, 3,0>();
    const auto resrl = fliprl - ProjectionMatrix{ 4, 3, 2, 1,
                                                  8, 7, 6, 5,
                                                  12,11,10,9 };
    QVERIFY(qFuzzyIsNull(resrl.norm()));

    const auto cornerElem = Pmat.subMat<2,2, 3,3>();
    QCOMPARE(cornerElem.ref(), 12.0);
    QCOMPARE(cornerElem, 12.0);
}
