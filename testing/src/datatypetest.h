#ifndef DATATYPETEST_H
#define DATATYPETEST_H

#include <QtTest>

class DataTypeTest : public QObject
{
    Q_OBJECT
public:
    DataTypeTest() = default;

private Q_SLOTS:
    void testChunkOperations();
    void testChunkMemAlloc();
    void testSetData();
    void testVoxelVolume();
    void testVoxelSlicing();
    void testVoxelFactory();
    void testVoxelReslicing();
    void testVoxelSizeChecks();
    void testVoxelMinMax();
    void testVoxelOperations();
    void testProjectionData();
};

#endif // DATATYPETEST_H
