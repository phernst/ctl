#include "datatypetest.h"

#include "img/chunk2d.h"
#include "img/voxelvolume.h"
#include "img/projectiondata.h"
#include "img/compositevolume.h"
#include "models/tabulateddatamodel.h"

using namespace CTL;


void DataTypeTest::testChunkOperations()
{
    const std::vector<float> testData(100,42.0f);

    Chunk2D<float> testChunk(10, 10, testData);
    Chunk2D<float> oprtChunk(10, 10, testData);

    Chunk2D<float> failChunk(10,11);

    // test addition
    auto resultChunk = testChunk + oprtChunk;
    QCOMPARE(resultChunk(0,0), 84.0f);
    QVERIFY_EXCEPTION_THROWN(testChunk + failChunk, std::domain_error);

    // test subtraction
    resultChunk = testChunk - oprtChunk;
    QVERIFY(qFuzzyIsNull(resultChunk(0,0)));
    QVERIFY_EXCEPTION_THROWN(testChunk - failChunk, std::domain_error);

    // test multiplication
    resultChunk = testChunk * 3.0f;
    QCOMPARE(resultChunk(0,0), 126.0f);

    // test division
    resultChunk = testChunk / 3.0f;
    QCOMPARE(resultChunk(0,0), 14.0f);

    // test comparison
    QVERIFY(testChunk == oprtChunk);
    QVERIFY(testChunk != resultChunk);
}

void DataTypeTest::testChunkMemAlloc()
{
    // test non-allocating instantiation
    Chunk2D<float> emptyChunk(10,10);
    QCOMPARE(emptyChunk.allocatedElements(), size_t(0));

    // test manual memory allocation
    emptyChunk.allocateMemory();
    QCOMPARE(emptyChunk.allocatedElements(), size_t(100));

    // test allocating instantiation
    Chunk2D<float> preallocChunk(10,10,42.0f);
    QCOMPARE(preallocChunk.allocatedElements(), size_t(100));
}

void DataTypeTest::testSetData()
{
    Chunk2D<float> testChunk(10,10);
    const std::vector<float> testData(100,42.0f);
    const std::vector<float> failData(101,42.0f);

    // test data setter
    testChunk.setData(testData);
    QCOMPARE(testChunk(0,0), 42.0f);

    // test exception when trying to set dimension-mismatching data
    QVERIFY_EXCEPTION_THROWN(testChunk.setData(failData), std::domain_error);
}

void DataTypeTest::testVoxelVolume()
{
    VoxelVolume<float> testVol(10,10,10);

    // test size definition
    QCOMPARE(testVol.totalVoxelCount(), size_t(1000));

    // test allocation
    QCOMPARE(testVol.hasData(), false);
    QCOMPARE(testVol.allocatedElements(), size_t(0));
    testVol.allocateMemory();
    QCOMPARE(testVol.hasData(), true);
    QCOMPARE(testVol.allocatedElements(), size_t(1000));

    // test filling (with pre-allocated memory)
    testVol.fill(42.0f);
    QCOMPARE(testVol.constData().at(0), 42.0f);
}

void DataTypeTest::testVoxelSlicing()
{
    VoxelVolume<int>::Dimensions dim = {2, 3, 4};

    VoxelVolume<int> testVol(dim);

    // prepare test data
    /*  z=0     z=1     z=2     z=3
     *  1  2    7  8   13 14   19 20
     *  3  4    9 10   15 16   21 22
     *  5  6   11 12   17 18   23 24  */
    std::vector<int> dataVec(testVol.totalVoxelCount());

    int dataIdx = 0;
    for(uint x=0; x<dim.x; ++x)
        for(uint y=0; y<dim.y; ++y)
            for(uint z=0; z<dim.z; ++z, ++dataIdx)
                dataVec[dataIdx] = dataIdx+1;

    testVol.setData(std::move(dataVec));

    // slicing: X direction
    /* requested result: slice x=0
     *  z=0     z=1     z=2     z=3
     *  1 .     7 .    13 .    19 .
     *  3 .     9 .    15 .    21 .
     *  5 .    11 .    17 .    23 .   */
    Chunk2D<int>::Dimensions reqDimXSlice = {3, 4};
    std::vector<int> reqResXSlice = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23};
    Chunk2D<int> xSlice = testVol.sliceX(0);
    QVERIFY(xSlice.dimensions() == reqDimXSlice);
    QVERIFY(xSlice.constData() == reqResXSlice);

    // slicing: Y direction
    /* requested result: slice y=1
     *  z=0     z=1     z=2     z=3
     *  .  .    .  .    .  .    .  .
     *  3  4    9 10   15 16   21 22
     *  .  .    .  .    .  .    .  .  */
    Chunk2D<int>::Dimensions reqDimYSlice = {2, 4};
    std::vector<int> reqResYSlice = {3, 4, 9, 10, 15, 16, 21, 22};
    Chunk2D<int> ySlice = testVol.sliceY(1);
    QVERIFY(ySlice.dimensions() == reqDimYSlice);
    QVERIFY(ySlice.constData() == reqResYSlice);

    // slicing: Z direction
    /* requested result: slice z=2
     *  z=0     z=1     z=2     z=3
     *  .  .    .  .    .  .   13 14
     *  .  .    .  .    .  .   15 16
     *  .  .    .  .    .  .   17 18  */
    Chunk2D<int>::Dimensions reqDimZSlice = {2, 3};
    std::vector<int> reqResZSlice = {13, 14, 15, 16, 17, 18};
    Chunk2D<int> zSlice = testVol.sliceZ(2);
    QVERIFY(zSlice.dimensions() == reqDimZSlice);
    QVERIFY(zSlice.constData() == reqResZSlice);


}

void DataTypeTest::testVoxelFactory()
{
    // prepare test data
    /*  (ch...chunk)
     *  ch=0    ch=1    ch=2    ch=3
     *  1  1    2  2    3  3    4  4
     *  1  1    2  2    3  3    4  4
     *  1  1    2  2    3  3    4  4  */
    VoxelVolume<int>::Dimensions dim = {2, 3, 4};
    std::vector<Chunk2D<int>> chunkStack;
    for(uint ch=0; ch<dim.z; ++ch)
        chunkStack.emplace_back(dim.x, dim.y, std::vector<int>(dim.x*dim.y,ch+1));

    // check volume fusing
    std::vector<int> reqRes;
    for(uint i=0; i<dim.z; ++i)
        for(uint j=0; j<dim.x*dim.y; ++j)
            reqRes.push_back(i+1);
    VoxelVolume<int> fusedVol = VoxelVolume<int>::fromChunk2DStack(chunkStack);
    QCOMPARE(fusedVol.nbVoxels().x, dim.x);
    QCOMPARE(fusedVol.nbVoxels().y, dim.y);
    QCOMPARE(fusedVol.nbVoxels().z, dim.z);
    QVERIFY(fusedVol.constData() == reqRes);

    // check exception
    chunkStack.push_back(Chunk2D<int>(dim.x, dim.x+dim.y+dim.z));
    QVERIFY_EXCEPTION_THROWN(VoxelVolume<int>::fromChunk2DStack(chunkStack), std::domain_error);
}

void DataTypeTest::testVoxelReslicing()
{
    VoxelVolume<int>::Dimensions dim = {2, 3, 4};

    VoxelVolume<int> testVol(dim);

    // prepare test data
    /*  z=0     z=1     z=2     z=3
     *  1  2    7  8   13 14   19 20
     *  3  4    9 10   15 16   21 22
     *  5  6   11 12   17 18   23 24  */
    std::vector<int> dataVec(testVol.totalVoxelCount());

    int dataIdx = 0;
    for(uint x=0; x<dim.x; ++x)
        for(uint y=0; y<dim.y; ++y)
            for(uint z=0; z<dim.z; ++z, ++dataIdx)
                dataVec[dataIdx] = dataIdx+1;

    testVol.setData(std::move(dataVec));

    // reslice volume in x-direction.
    VoxelVolume<int> reslicedInX = testVol.reslicedByX();
    QCOMPARE(reslicedInX.nbVoxels().x, dim.y);
    QCOMPARE(reslicedInX.nbVoxels().y, dim.z);
    QCOMPARE(reslicedInX.nbVoxels().z, dim.x); // x becomes new z dim.
    /* We check for these [x] elements.
     *  z=0     z=1     z=2     z=3
     * [1][2]   7  8   13 14   19 20
     *  3  4    9 10   15 16   21[22]
     *  5  6   11[12]  17 18   23 24  */
    QCOMPARE(reslicedInX(0,0,0), 1);
    QCOMPARE(reslicedInX(0,0,1), 2);
    QCOMPARE(reslicedInX(2,1,1), 12);
    QCOMPARE(reslicedInX(1,3,1), 22);


    // reslice volume in reversed y-direction.
    VoxelVolume<int> reslicedInY = testVol.reslicedByY(true);
    QCOMPARE(reslicedInY.nbVoxels().x, dim.x);
    QCOMPARE(reslicedInY.nbVoxels().y, dim.z);
    QCOMPARE(reslicedInY.nbVoxels().z, dim.y); // y becomes new z dim.
    /* We check for these [x] elements.
     *  z=0     z=1     z=2     z=3
     *  1  2    7  8   13 14   19[20]
     * [3] 4    9 10   15[16]  21 22
     * [5] 6   11 12   17 18   23 24  */
    QCOMPARE(reslicedInY(0,0,1), 3);
    QCOMPARE(reslicedInY(0,0,0), 5);
    QCOMPARE(reslicedInY(1,2,1), 16);
    QCOMPARE(reslicedInY(1,3,2), 20);


    // reslice volume in reversed z-direction.
    VoxelVolume<int> reslicedInZRev = testVol.reslicedByZ(true);
    QCOMPARE(reslicedInZRev.nbVoxels().x, dim.x);
    QCOMPARE(reslicedInZRev.nbVoxels().y, dim.y);
    QCOMPARE(reslicedInZRev.nbVoxels().z, dim.z);
    /* We check for these [x] elements.
     *  z=0     z=1     z=2     z=3
     *  1  2    7  8  [13]14  [19]20
     *  3 [4]   9 10   15 16   21 22
     *  5  6   11 12   17[18]  23 24  */
    QCOMPARE(reslicedInZRev(1,1,3), 4);
    QCOMPARE(reslicedInZRev(0,0,1), 13);
    QCOMPARE(reslicedInZRev(1,2,1), 18);
    QCOMPARE(reslicedInZRev(0,0,0), 19);


    // reslice volume in z-direction.
    VoxelVolume<int> reslicedInZ = testVol.reslicedByZ();
    /* Should not change anything!  */
    QVERIFY(reslicedInZ.constData() == testVol.constData());
}

void DataTypeTest::testVoxelSizeChecks()
{
    VoxelVolume<int>::Dimensions dim = { 10, 10, 10 };
    VoxelVolume<int>::VoxelSize voxSizeCube = { 1.1f, 1.1f, 1.1f };
    VoxelVolume<int>::VoxelSize voxSizeRect1 = { 2.2f, 1.1f, 3.3f };
    VoxelVolume<int>::VoxelSize voxSizeRect2 = { 3.3f, 2.2f, 2.2f};
    VoxelVolume<int>::VoxelSize voxSizeRect3 = { 3.3f, 3.3f, 2.2f };
    VoxelVolume<int> cubeVol(dim, voxSizeCube);
    VoxelVolume<int> rectVol1(dim, voxSizeRect1);
    VoxelVolume<int> rectVol2(dim, voxSizeRect2);
    VoxelVolume<int> rectVol3(dim, voxSizeRect3);

    QCOMPARE(cubeVol.smallestVoxelSize(), 1.1f);
    QCOMPARE(rectVol1.smallestVoxelSize(), 1.1f);
    QCOMPARE(rectVol2.smallestVoxelSize(), 2.2f);
    QCOMPARE(rectVol3.smallestVoxelSize(), 2.2f);
}

void DataTypeTest::testVoxelMinMax()
{
    VoxelVolume<float> testVol(10,10,10);

    QCOMPARE(testVol.min(), 0.0f);
    QCOMPARE(testVol.max(), 0.0f);

    float testValue = 1337.0f;
    testVol.fill(testValue);

    QCOMPARE(testVol.min(), testValue);
    QCOMPARE(testVol.max(), testValue);

    float testValue2 = -1337.0f;
    testVol(3,4,5) = testValue2;

    QCOMPARE(testVol.min(), testValue2);
    QCOMPARE(testVol.max(), testValue);
}

void DataTypeTest::testVoxelOperations()
{
    VoxelVolume<float> testVol1(10,10,10), testVol2(10,10,10);
    testVol1.fill(3.0f);
    testVol2.fill(2.0f);

    QCOMPARE((testVol1 + testVol2)(0,0,0), 5.0f);
    QCOMPARE((testVol2 + testVol1)(0,0,0), 5.0f);
    QCOMPARE((testVol1 - testVol2)(0,0,0), 1.0f);
    QCOMPARE((testVol2 - testVol1)(0,0,0), -1.0f);
    QCOMPARE((testVol1 + 2.0f)(0,0,0), 5.0f);
    QCOMPARE((testVol1 - 2.0f)(0,0,0), 1.0f);
    QCOMPARE((testVol1 * 2.0f)(0,0,0), 6.0f);
    QCOMPARE((testVol2 / 2.0f)(0,0,0), 1.0f);

    // in-place operands
    testVol1 += testVol2;
    QCOMPARE(testVol1(0,0,0), 5.0f);
    testVol1 -= testVol2;
    QCOMPARE(testVol1(0,0,0), 3.0f);
    testVol1 += 2.0f;
    QCOMPARE(testVol1(0,0,0), 5.0f);
    testVol1 -= 2.0f;
    QCOMPARE(testVol1(0,0,0), 3.0f);
    testVol1 *= 2.0f;
    QCOMPARE(testVol1(0,0,0), 6.0f);
    testVol1 /= 2.0f;
    QCOMPARE(testVol1(0,0,0), 3.0f);

    // test exception handling
    VoxelVolume<float> exceptVol(10,10,11);
    exceptVol.fill(1.0f);
    QVERIFY_EXCEPTION_THROWN(testVol1 + exceptVol, std::domain_error);
    QVERIFY_EXCEPTION_THROWN(testVol1 - exceptVol, std::domain_error);
    QVERIFY_EXCEPTION_THROWN(testVol1 += exceptVol, std::domain_error);
    QVERIFY_EXCEPTION_THROWN(testVol1 -= exceptVol, std::domain_error);

}

void DataTypeTest::testProjectionData()
{
    SingleViewData::Dimensions svDim = { 10, 10, 5 };
    ProjectionData testProj(svDim);

    const uint nbViews = 2;
    std::vector<float> testData(svDim.nbChannels * svDim.nbRows * svDim.nbModules * nbViews);
    std::iota(testData.begin(),testData.end(), 1.0f);

    testProj.setDataFromVector(testData);

    QCOMPARE(testProj.nbViews(), nbViews);
    QVERIFY(testProj.viewDimensions() == svDim);

    QCOMPARE(testProj.view(0).module(0)(0,0), 1.0f);
    QCOMPARE(testProj.view(0).module(2)(0,0), 201.0f);
    QCOMPARE(testProj.view(1).module(0)(0,0), 501.0f);
    QCOMPARE(testProj.view(1).module(0)(2,1), 513.0f);

    // single view with one module init
    SingleViewData singleViewData(testProj.view(0).module(0));
    QCOMPARE(singleViewData.dimensions().nbChannels, svDim.nbChannels);
    QCOMPARE(singleViewData.dimensions().nbRows, svDim.nbRows);
    QCOMPARE(singleViewData.dimensions().nbModules, 1u);

    // implicit construction of a single module
    ProjectionData zeroInitializedModule{ { { 640, 480, 0.0f } } };
    QCOMPARE(zeroInitializedModule.dimensions().nbChannels, 640u);
    QCOMPARE(zeroInitializedModule.dimensions().nbRows, 480u);
    QCOMPARE(zeroInitializedModule.dimensions().nbModules, 1u);
    QCOMPARE(zeroInitializedModule.dimensions().nbViews, 1u);
    for(auto val : zeroInitializedModule.toVector())
        QCOMPARE(val, 0.0f);
}

void DataTypeTest::testCompositeVolume()
{
    VoxelVolume<float> vol1(10,10,10);
    vol1.fill(1.0f);

    auto tabModel = std::make_shared<TabulatedDataModel>();
    tabModel->insertDataPoint(1.0, 1.0);
    tabModel->insertDataPoint(2.0, 2.0);
    tabModel->insertDataPoint(3.0, 3.0);

    auto tabModel2 = std::make_shared<TabulatedDataModel>();
    tabModel2->insertDataPoint(1.0, 2.0);
    tabModel2->insertDataPoint(2.0, 4.0);
    tabModel2->insertDataPoint(3.0, 6.0);

    SpectralVolumeData realVol1(vol1, tabModel, "boy");
    SpectralVolumeData realVol2(vol1, tabModel2, "heavy boy");

    QCOMPARE(realVol1.meanMassAttenuationCoeff(1.5f, 1.0f), 1.5f);
    QCOMPARE(realVol1.muVolume(1.5f, 1.0f)->max(), 0.15f);

    QCOMPARE(realVol2.meanMassAttenuationCoeff(1.5f, 1.0f), 3.0f);
    QCOMPARE(realVol2.muVolume(1.5f, 1.0f)->max(), 0.3f);

    CompositeVolume compositeVol;
    compositeVol.addSubVolume(realVol1);
    compositeVol.addSubVolume(realVol2);

    QCOMPARE(compositeVol.nbSubVolumes(), 2u);
    QCOMPARE(compositeVol.muVolume(0, 1.5f, 1.0f)->max(), 0.15f);
    QCOMPARE(compositeVol.muVolume(1, 1.5f, 1.0f)->max(), 0.30f);
}
