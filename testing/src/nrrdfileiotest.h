#ifndef NRRDFILEIOTEST_H
#define NRRDFILEIOTEST_H

#include "acquisition/viewgeometry.h"
#include "img/projectiondata.h"
#include "img/voxelvolume.h"
#include <QtTest>

class NrrdFileIOtest : public QObject
{
    Q_OBJECT

public:
    NrrdFileIOtest() = default;

private Q_SLOTS:
    //void initTestCase();
    void testMetaInfo();
    void testFields();
    void testHeaderProperties();

private:

//    CTL::FullGeometry _testGeometry;
//    CTL::ProjectionData _testProjections = CTL::ProjectionData(0, 0, 0);
//    CTL::VoxelVolume<float> _testVolume = CTL::VoxelVolume<float>(0, 0, 0);
};

#endif // NRRDFILEIOTEST_H
