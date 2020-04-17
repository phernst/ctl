#ifndef GEOMETRYTEST_H
#define GEOMETRYTEST_H

#include <QtTest>
#include "acquisition/viewgeometry.h"
#include "mat/mat.h"

namespace CTL {
    class CTSystem;
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
    void testDecoderEncoderConsistency();
    void cleanupTestCase();

private:
    CTL::CTSystem* _cArmTestSystem;
    CTL::CTSystem* _tubeTestSystem;

    void verifyPmatDiff(const CTL::FullGeometry& toVerify, const CTL::FullGeometry& original);

};

#endif // GEOMETRYTEST_H
