#ifndef DENFILEIOTEST_H
#define DENFILEIOTEST_H

#include <QtTest>
#include "img/projectiondata.h"
#include "img/voxelvolume.h"
#include "io/abstractbasetypeio.h"
#include "acquisition/viewgeometry.h"

class DenFileIOtest : public QObject
{
    Q_OBJECT

public:
    DenFileIOtest() = default;

private Q_SLOTS:
    void initTestCase();
    void testPmatReader();
    void testPolicyBasedIO();
    void testModuleCount();
    void testAbstractInterface();

private:
    void verifyFullGeoDiff(const CTL::FullGeometry &toVerify, const CTL::FullGeometry &original, double tolerance);
    double viewGeoDiff(const CTL::SingleViewGeometry &toVerify, const CTL::SingleViewGeometry &original);
    void verifyProjDiff(const CTL::ProjectionData &toVerify, const CTL::ProjectionData &original, double tolerance);
    template <typename T>
    void verifyVolumeDiff(const CTL::VoxelVolume<T>& toVerify, const CTL::VoxelVolume<T>& original, T tolerance);
    void oneModuleOneView(const CTL::mat::ProjectionMatrix& pMat, const CTL::ProjectionData& projImages);
    void oneModuleMultipleViews(const CTL::mat::ProjectionMatrix& pMat, const CTL::ProjectionData& projImages);
    void oneViewMultipleModules_geo(const CTL::mat::ProjectionMatrix& pMat);
    void processAbstractVolume(CTL::io::AbstractVolumeIO<float>* volIO, const QString& fileName);
    void processAbstractProjDat(CTL::io::AbstractProjectionDataIO* projDatIO, const QString& fileName);
    void processAbstractProjMat(CTL::io::AbstractProjectionMatrixIO* projMatIO, const QString& fileName);

    CTL::FullGeometry _testGeometry;
    CTL::ProjectionData _testProjections = CTL::ProjectionData(0, 0, 0);
    CTL::VoxelVolume<float> _testVolume = CTL::VoxelVolume<float>(0, 0, 0);
};

#endif // DENFILEIOTEST_H
