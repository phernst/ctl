#ifndef TST_PROJECTIONSMATRIX_H
#define TST_PROJECTIONSMATRIX_H

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

private:
    CTL::mat::ProjectionMatrix P;
};

#endif // TST_PROJECTIONSMATRIX_H
