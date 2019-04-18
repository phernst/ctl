#ifndef PROJECTORTEST_H
#define PROJECTORTEST_H

#include "img/chunk2d.h"
#include "img/voxelvolume.h"
#include "img/projectiondata.h"
#include "io/den/den.h"

#include <QtTest>

class ProjectorTest : public QObject
{
    Q_OBJECT

public:
    ProjectorTest() = default;

private Q_SLOTS:
    void initTestCase();
    void testPoissonExtension();
    void testSpectralExtension();

private:
    CTL::VoxelVolume<float> _testVolume = CTL::VoxelVolume<float>(0,0,0);

    void poissonSimulation(double meanPhotons, double projAngle, uint nbRepetitions) const;
    void evaluatePoissonSimulation(const CTL::ProjectionData &repeatedProjs,
                                   const CTL::ProjectionData& cleanProjections,
                                   double intensity) const;

    // helper methods
    double countsMean(const CTL::ProjectionData& projections, double i_0) const;
    double countsVariance(const CTL::ProjectionData& projections, double i_0) const;
    double projectionMean(const CTL::ProjectionData& projections) const;
    double projectionVariance(const CTL::ProjectionData& projections) const;

    CTL::Chunk2D<double> repetitionMean(const CTL::ProjectionData& repeatedProjs, double i_0) const;
    CTL::Chunk2D<double> repetitionVariance(const CTL::ProjectionData& repeatedProjs, double i_0) const;
    CTL::Chunk2D<double> transformedToCounts(const CTL::SingleViewData::ModuleData& module, double i_0) const;
    CTL::VoxelVolume<float>::Dimensions toVoxelVolumeDimensions(const CTL::io::den::Header& header);
    template<typename T>
    double chunkMean(const CTL::Chunk2D<T>& chunk) const;
};

#endif // PROJECTORTEST_H
