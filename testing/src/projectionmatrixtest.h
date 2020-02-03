#ifndef TST_PROJECTIONSMATRIX_H
#define TST_PROJECTIONSMATRIX_H

#define ENABLE_FROBENIUS_NORM
#include "mat/projectionmatrix.h"
#include <QtTest>

class ProjectionMatrixTest : public QObject
{
    Q_OBJECT

public:
    ProjectionMatrixTest() = default;

private Q_SLOTS:
    void init();
    void initTestCase();
    void principalRay();
    void sourcePosition();
    void resampleDetector();
    void shiftOrigin();
    void skewCoefficient();
    void projectionOntoDetector();
    void equalityTest();
    void comparatorTest();
    void subMatExtraction();

private:
    CTL::mat::ProjectionMatrix P;
};

#endif // TST_PROJECTIONSMATRIX_H
