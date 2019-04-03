#ifndef GEOMETRYTEST_H
#define GEOMETRYTEST_H

#include <QtTest>
#include "acquisition/viewgeometry.h"
#include "mat/mat.h"

namespace CTL {
    class CTsystem;
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

private:
    CTL::CTsystem* _cArmTestSystem;
    CTL::CTsystem* _tubeTestSystem;

    void verifyPmatDiff(const CTL::FullGeometry& toVerify, const CTL::FullGeometry& original);

};

#endif // GEOMETRYTEST_H
