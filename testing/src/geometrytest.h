#ifndef GEOMETRYTEST_H
#define GEOMETRYTEST_H

#include <QtTest>
#include "mat/mat.h"

namespace CTL {
    class CTsystem;
    typedef QVector<ProjectionMatrix> SingleViewGeometry;
    typedef QVector<SingleViewGeometry> FullGeometry;
}

class GeometryTest : public QObject
{
    Q_OBJECT
public:
    GeometryTest() = default;

private Q_SLOTS:
    void initTestCase();
    void testGeometryDecoder();
    void testGeometryEncoder();

private:
    CTL::CTsystem* _cArmTestSystem;
    CTL::CTsystem* _tubeTestSystem;

    void verifyPmatDiff(const CTL::FullGeometry& toVerify, const CTL::FullGeometry& original);

};

#endif // GEOMETRYTEST_H
