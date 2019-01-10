#include "projectionmatrixtest.h"
#include "mat/mat.h"

using namespace CTL::mat;

// clang-format off
static const ProjectionMatrix Pinit =
ProjectionMatrix::compose(Matrix<3,3>(
                          { 1000.0,    0.0, 300.0,
                               0.0, 1000.0, 250.0,
                               0.0,    0.0,   1.0 }),
                          rotationMatrix(-90.0_deg, Qt::XAxis),
                          Matrix<3,1>({ 0.0, 300.0, 0.0 }));
// clang-format on

// print infos at beginning
void ProjectionMatrixTest::initTestCase()
{
    /*
    init();
    qDebug().noquote() << "use projection matrix:\n"
                       << QString::fromStdString(P.info());
    */
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
    QVERIFY(qFuzzyIsNull((Matrix<3, 1>({ 0.0, 300.0, 0.0 }) - P.sourcePosition()).norm()));
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
    auto homoMat = ProjectionMatrix::compose(Matrix<3,3>(
                                            { 1000.0,    0.5, 300.0,
                                                 0.0, 1000.0, 250.0,
                                                 0.0,    0.0,   1.0 }),
                                            Matrix<3,1>(0.0));
    QCOMPARE(homoMat.skewCoefficient(), 0.5);
    QCOMPARE(P.skewCoefficient(), 0.0);
}

void ProjectionMatrixTest::projectionOntoDetector()
{
    Matrix<3, 1> testVec({ 1.0, 4.0, 8.0 });
    auto res1 = P * vertcat(testVec, Matrix<1, 1>(1.0));
    res1 /= res1(2);

    auto res2 = P.projectOntoDetector(testVec);

    QCOMPARE(res2(0), res1(0));
    QCOMPARE(res2(1), res1(1));
}

void ProjectionMatrixTest::equalityTest()
{
    // clang-format off
    Matrix<2, 3> matA({ 1.0, 2.0, 3.0,
                        1.0, 2.0, 3.0 });
    Matrix<2, 3> minusMatA({ -1.0, -2.0, -3.0,
                             -1.0, -2.0, -3.0 });
    Matrix<2, 3> matB({ 1.0, 2.0, 3.0,
                        1.0, 2.0, 3.0 });
    Matrix<2, 3> matC({ 1.0, 2.0, 3.0,
                        1.0, 2.01, 3.0 });
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
