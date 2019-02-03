#define ENABLE_FROBENIUS_NORM

#include "denfileiotest.h"

#include "components/allcomponents.h"
#include "acquisition/geometryencoder.h"
#include "acquisition/systemblueprints.h"
#include "acquisition/trajectories.h"
#include "mat/mat.h"

#include "io/basetypeio.h"
#include "io/den/denfileio.h"


using namespace CTL;


void DenFileIOtest::initTestCase()
{
    const uint nbViews = 3;
    const VoxelVolume<float>::Dimensions volDim { 20, 10, 15 };
    const SingleViewData::Dimensions viewDim { 6, 7, 5 };
    const SingleViewData::ModuleData::Dimensions moduleDim { viewDim.nbChannels, viewDim.nbRows };

    // build test data
    VoxelVolume<float> vol(volDim);
    vol.fill(13.37f);

    ProjectionData projs(viewDim);
    SingleViewData::ModuleData modDat(moduleDim);

    for(uint view = 0; view< nbViews; ++view)
    {
        SingleViewData viewDat(moduleDim);
        for(uint mod = 0; mod< viewDim.nbModules; ++mod)
        {
            modDat.fill(static_cast<float>(mod + view * viewDim.nbModules));
            viewDat.append(modDat);
        }
        projs.append(viewDat);
    }

    FullGeometry fullGeo;

    qsrand(uint(QTime::currentTime().msecsSinceStartOfDay()));
    for(uint view = 0; view< nbViews; ++view)
    {
        SingleViewGeometry viewGeo;
        for(uint mod = 0; mod< viewDim.nbModules; ++mod)
        {
            std::vector<double> pMat(12);
            for(uint val = 0; val < 12u; ++val)
                pMat[val] = static_cast<double>(qrand()) / RAND_MAX;
            viewGeo.append(mat::ProjectionMatrix::fromContainer(pMat,0));
        }
        fullGeo.append(viewGeo);
    }

    _testVolume = vol;
    _testProjections = projs;
    _testGeometry = fullGeo;
}

void DenFileIOtest::testPmatReader()
{
    CTsystem system = CTsystemBuilder::createFromBlueprint(blueprints::GenericTubularCT());

    // create an acquisition setup (here: helical scan protocol)
    AcquisitionSetup mySetup(system);
    mySetup.setNbViews(1);
    mySetup.applyPreparationProtocol(protocols::HelicalTrajectory(10.0_deg, 0.0));

    auto geo = GeometryEncoder::encodeFullGeometry(mySetup);

    QVector<double> vectorizedPmats;
    foreach (const SingleViewGeometry& sViewGeo, geo)
        foreach (const ProjectionMatrix& pmat, sViewGeo)
            vectorizedPmats.append(mat::toQVector(pmat));

    const QString fName("testData/geometryConv.den");

    io::den::QDFile outFile(fName);
    outFile.setVerbose(false);
    outFile.save(vectorizedPmats, 3, 4);

    io::BaseTypeIO<io::DenFileIO> fileReader;

    auto pMats = fileReader.readFullGeometry(fName, 40);

    verifyFullGeoDiff(pMats, geo, 1.0e-8);
}

void DenFileIOtest::verifyFullGeoDiff(const FullGeometry &toVerify, const FullGeometry &original, double tolerance)
{
    double sumDiff = 0.0;

    auto nbViews = toVerify.length();

    for(auto view = 0; view < nbViews; ++view)
        sumDiff += viewGeoDiff(toVerify.at(view), original.at(view));

    auto normalizedDiff = sumDiff / double(nbViews);

    //qInfo() << "diff: " << normalizedDiff;
    QVERIFY(normalizedDiff <= tolerance);
}

double DenFileIOtest::viewGeoDiff(const SingleViewGeometry &toVerify, const SingleViewGeometry &original)
{
    double sumDiff = 0.0;

    auto nbModules = toVerify.length();

        for(auto mod = 0; mod < nbModules; ++mod)
        {
            auto diff = toVerify.at(mod).normalized() - original.at(mod).normalized();
            sumDiff += diff.norm();
        }

    auto normalizedDiff = sumDiff / double(nbModules);

    return normalizedDiff;
}

void DenFileIOtest::verifyProjDiff(const ProjectionData &toVerify, const ProjectionData &original, double tolerance)
{
    double sumDiff = 0.0;

    auto diffProjs = toVerify - original;

    auto vectorized = diffProjs.toVector();

    for(const auto& pix : vectorized)
        sumDiff += double(fabs(pix));

    auto normalizedDiff = sumDiff / double(vectorized.size());

    //qInfo() << "diff: " << normalizedDiff;
    QVERIFY(normalizedDiff <= tolerance);
}

template<typename T>
void DenFileIOtest::verifyVolumeDiff(const VoxelVolume<T>& toVerify, const VoxelVolume<T>& original, T tolerance)
{
    double sumDiff = 0.0;

    auto nbVoxels = toVerify.totalVoxelCount();

    auto diffVol = toVerify - original;

    for(const auto& vox : diffVol.constData())
        sumDiff += fabs(vox);

    auto normalizedDiff = sumDiff / double(nbVoxels);

    //qInfo() << "diff: " << normalizedDiff;
    QVERIFY(normalizedDiff <= static_cast<double>(tolerance));
}

void DenFileIOtest::testPolicyBasedIO()
{
    io::BaseTypeIO<io::DenFileIO> fileHandler;

    // write files
    QVERIFY(fileHandler.write(_testGeometry, "testData/geometrySave.den"));
    QVERIFY(fileHandler.write(_testProjections, "testData/projectionsSave.den"));
    QVERIFY(fileHandler.write(_testVolume, "testData/volumeSave.den"));

    // re-load stored data
    uint nbModules = _testProjections.viewDimensions().nbModules;
    auto loadedVol = fileHandler.readVolume<float>("testData/volumeSave.den");
    auto loadedPmats = fileHandler.readFullGeometry("testData/geometrySave.den", nbModules);
    auto loadedProjs = fileHandler.readProjections("testData/projectionsSave.den", nbModules);

    const uint testView = 1;
    const uint testModule = 2;
    const uint testSlice = 14;
    auto loadedSlice = fileHandler.readSlice<float>("testData/volumeSave.den", testSlice);
    auto loadedSingleProj = fileHandler.readSingleView("testData/projectionsSave.den", testView, nbModules);
    auto loadedViewPmats = fileHandler.readSingleViewGeometry("testData/geometrySave.den",testView, nbModules);

    // evaluate
    verifyVolumeDiff(loadedVol, _testVolume, 0.0f);
    verifyFullGeoDiff(loadedPmats, _testGeometry, 0.0);
    verifyProjDiff(loadedProjs, _testProjections, 0.0);

    QCOMPARE(viewGeoDiff(loadedViewPmats, _testGeometry.at(testView)), 0.0);
    QVERIFY(loadedSlice == _testVolume.sliceZ(testSlice));
    QVERIFY(loadedSingleProj.dimensions() == _testProjections.viewDimensions());
    QCOMPARE(loadedSingleProj.module(testModule)(0,0) , _testProjections.view(testView).module(testModule)(0,0));
}

// This test checks the fallback behavior if number modules is not specified and not in the meta info of the file
void DenFileIOtest::testModuleCount()
{
    ProjectionMatrix pMat;
    std::iota(pMat.begin(), pMat.end(), 0.0);

    ProjectionData projImages(64, 16, 1);
    projImages.allocateMemory(1);
    std::iota(projImages.view(0).module(0).rawData(), projImages.view(0).module(0).rawData() + 64*16, 0.0f);

    try {
        // one module | one view
        oneModuleOneView(pMat, projImages);
        oneModuleMultipleViews(pMat, projImages);

        // one module | n views
        projImages.append(projImages.view(0) * 1.5f);
        oneModuleMultipleViews(pMat, projImages); // 2 views
        projImages.append(projImages.view(1) * 1.5f);
        oneModuleMultipleViews(pMat, projImages); // 3 views
        projImages.append(projImages.view(2) * 1.5f);
        oneModuleMultipleViews(pMat, projImages); // 4 views

        // two modules | one view
        oneViewMultipleModules_geo(pMat);

    } catch (const std::exception& e) {
        qInfo() << e.what();
        QVERIFY(false);
    }
}

void DenFileIOtest::testAbstractInterface()
{
    using io::BaseTypeIO;
    using io::DenFileIO;

    auto volIO = BaseTypeIO<DenFileIO>::makeVolumeIO<float>();
    processAbstractVolume(volIO.get(), "testData/abstractVolIO.den");

    auto projDatIO = BaseTypeIO<DenFileIO>::makeProjectionDataIO();
    processAbstractProjDat(projDatIO.get(), "testData/abstractProjDatIO.den");

    auto projMatIO = BaseTypeIO<DenFileIO>::makeProjectionMatrixIO();
    processAbstractProjMat(projMatIO.get(), "testData/abstractProjMatIO.den");
}

void DenFileIOtest::oneModuleOneView(const ProjectionMatrix& pMat, const ProjectionData& projImage)
{
    // projection matrices
    io::BaseTypeIO<io::DenFileIO> io;

    SingleViewGeometry sV{ { pMat } };
    FullGeometry fG{ sV };
    io.write(fG, "testData/1view_1module_geo.den");

    auto load_fG_1 = io.readFullGeometry("testData/1view_1module_geo.den");
    auto load_fG_2 = io.readFullGeometry("testData/1view_1module_geo.den", 1);
    auto load_sVG_1 = io.readSingleViewGeometry("testData/1view_1module_geo.den", 0);
    auto load_sVG_2 = io.readSingleViewGeometry("testData/1view_1module_geo.den", 0, 1);

    // check dimensions
    QCOMPARE(load_fG_1.at(0).length(), 1);
    QCOMPARE(load_fG_2.at(0).length(), 1);
    QCOMPARE(load_sVG_1.length(), 1);
    QCOMPARE(load_sVG_2.length(), 1);
    // check content
    verifyFullGeoDiff(load_fG_1, fG, 0.0);
    verifyFullGeoDiff(load_fG_2, fG, 0.0);
    QVERIFY(qFuzzyIsNull(viewGeoDiff(load_sVG_1, sV)));
    QVERIFY(qFuzzyIsNull(viewGeoDiff(load_sVG_2, sV)));

    // projection images
    io.write(projImage, "testData/1view_1module_img.den");
    auto load_pD_1 = io.readProjections("testData/1view_1module_img.den");
    auto load_pD_2 = io.readProjections("testData/1view_1module_img.den", 1);
    auto load_sV_1 = io.readSingleView("testData/1view_1module_img.den", 0);
    auto load_sV_2 = io.readSingleView("testData/1view_1module_img.den", 0, 1);

    // check dimensions
    QCOMPARE(load_pD_1.dimensions().nbModules, 1u);
    QCOMPARE(load_pD_2.dimensions().nbModules, 1u);
    QCOMPARE(load_sV_1.nbModules(), 1u);
    QCOMPARE(load_sV_2.nbModules(), 1u);
    // check content
    verifyProjDiff(load_pD_1, projImage, 0.0);
    verifyProjDiff(load_pD_2, projImage, 0.0);
    ProjectionData tmp1(load_sV_1.dimensions()); tmp1.append(load_sV_1);
    ProjectionData tmp2(load_sV_2.dimensions()); tmp2.append(load_sV_2);
    verifyProjDiff(tmp1, projImage, 0.0);
    verifyProjDiff(tmp2, projImage, 0.0);
}

void DenFileIOtest::oneModuleMultipleViews(const ProjectionMatrix& pMat, const ProjectionData& projImages)
{
    uint nbViews = projImages.nbViews();
    // projection matrices
    io::BaseTypeIO<io::DenFileIO> io;

    SingleViewGeometry sV{ { pMat } };
    FullGeometry fG;
    for(uint view = 0; view < nbViews; ++view)
    {
        sV[0] += ProjectionMatrix(view); // modify each view by a const offset
        fG.append(sV);
    }
    const QString fileNameGeo = "testData/" + QString::number(nbViews) + "views_1module_geo.den";
    io.write(fG, fileNameGeo);

    auto load_fG_1 = io.readFullGeometry(fileNameGeo);
    auto load_fG_2 = io.readFullGeometry(fileNameGeo, 1);
    FullGeometry load_sV1_geo;
    FullGeometry load_sV2_geo;
    for(uint view = 0; view < nbViews; ++view)
    {
        load_sV1_geo.append(io.readSingleViewGeometry(fileNameGeo, view));
        load_sV2_geo.append(io.readSingleViewGeometry(fileNameGeo, view, 1));
    }
    for(uint view = 0; view < nbViews; ++view)
    {
        QCOMPARE(load_fG_1.at(int(view)).length(), 1);
        QCOMPARE(load_fG_2.at(int(view)).length(), 1);
        QCOMPARE(load_sV1_geo.at(int(view)).length(), 1);
        QCOMPARE(load_sV2_geo.at(int(view)).length(), 1);
    }
    verifyFullGeoDiff(load_fG_1, fG, 0.0);
    verifyFullGeoDiff(load_fG_2, fG, 0.0);
    verifyFullGeoDiff(load_sV1_geo, fG, 0.0);
    verifyFullGeoDiff(load_sV2_geo, fG, 0.0);

    // projection images
    const QString fileNameImg = "testData/" + QString::number(nbViews) + "views_1module_img.den";
    io.write(projImages, fileNameImg);

    auto load_pD_1 = io.readProjections(fileNameImg);
    auto load_pD_2 = io.readProjections(fileNameImg, 1);

    auto sV1 = io.readSingleView(fileNameImg, 0);
    auto sV2 = io.readSingleView(fileNameImg, 0, 1);
    ProjectionData load_sV1_img(sV1.dimensions());
    load_sV1_img.append(std::move(sV1));
    ProjectionData load_sV2_img(sV2.dimensions());
    load_sV2_img.append(std::move(sV2));

    for(uint view = 1; view < nbViews; ++view)
    {
        load_sV1_img.append(io.readSingleView(fileNameImg, view));
        load_sV2_img.append(io.readSingleView(fileNameImg, view, 1));
    }
    // check dimensions
    QCOMPARE(load_pD_1.dimensions().nbModules, 1u);
    QCOMPARE(load_pD_2.dimensions().nbModules, 1u);
    QCOMPARE(load_sV1_img.dimensions().nbModules, 1u);
    QCOMPARE(load_sV2_img.dimensions().nbModules, 1u);

    // check content
    verifyProjDiff(load_pD_1, projImages, 0.0);
    verifyProjDiff(load_pD_2, projImages, 0.0);
    verifyProjDiff(load_sV1_img, projImages, 0.0);
    verifyProjDiff(load_sV2_img, projImages, 0.0);
}

void DenFileIOtest::oneViewMultipleModules_geo(const CTL::mat::ProjectionMatrix& pMat)
{
    FullGeometry fullGeo{ { 1.0 * pMat, 2.0 * pMat } }; // one view and two modules
    QVERIFY(fullGeo.length() == 1);
    QVERIFY(fullGeo.at(0).length() == 2);

    io::BaseTypeIO<io::DenFileIO> io;
    const QString fileNameGeo = "testData/oneView_multiModules_geo.den";
    io.write(fullGeo, fileNameGeo);

    auto nbModules = uint(fullGeo.at(0).length());
    auto load_fG_1 = io.readFullGeometry(fileNameGeo, nbModules); // correct case
    auto load_fG_2 = io.readFullGeometry(fileNameGeo); // wrong case (info about nbModules is missing)
    auto load_sVG_1 = io.readSingleViewGeometry(fileNameGeo, 0, nbModules); // correct case
    auto load_sVG_2 = io.readSingleViewGeometry(fileNameGeo, 0); // wrong case (info about nbModules is missing)
    auto load_sVG_3 = io.readSingleViewGeometry(fileNameGeo, 1); // wrong case -> "inventing" an additional view

    // check dimensions
    QCOMPARE(load_fG_1.at(0).length(), int(nbModules)); // correct case
    QCOMPARE(load_fG_2.at(0).length(), 1); // wrong case
    QCOMPARE(load_sVG_1.length(), int(nbModules)); // correct case
    QCOMPARE(load_sVG_2.length(), 1); // wrong case
    QCOMPARE(load_sVG_3.length(), 1); // wrong case (fake view)
    // check content
    verifyFullGeoDiff(load_fG_1, fullGeo, 0.0); // correct case
    QVERIFY(qFuzzyIsNull((load_fG_2.at(0).at(0) - fullGeo.at(0).at(0)).norm())); // wrong case
    QVERIFY(qFuzzyIsNull((load_fG_2.at(1).at(0) - fullGeo.at(0).at(1)).norm())); // wrong case (fake view)
    QVERIFY(qFuzzyIsNull(viewGeoDiff(load_sVG_1, fullGeo.at(0)))); // correct case
    QVERIFY(qFuzzyIsNull((load_sVG_2.at(0) - fullGeo.at(0).at(0)).norm())); // wrong case
    QVERIFY(qFuzzyIsNull((load_sVG_3.at(0) - fullGeo.at(0).at(1)).norm())); // wrong case (fake view)
}

void DenFileIOtest::processAbstractVolume(CTL::io::AbstractVolumeIO<float>* volIO, const QString &fileName)
{
    auto volume = _testVolume;
    volume(2,3,4) = volume(2,3,4) * 3.0f;

    QVERIFY(volIO->write(volume, fileName));

    auto metaInfo = volIO->metaInfo(fileName);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimX).toInt(), 20);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimY).toInt(), 10);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimZ).toInt(), 15);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::typeHint).toString(), CTL::io::meta_info::type_hint::volume);

    auto r = volIO->readSlice(fileName,4);
    QCOMPARE(r(2,3), volume(2,3,4));
}

void DenFileIOtest::processAbstractProjDat(io::AbstractProjectionDataIO* projDatIO, const QString &fileName)
{
    auto dims = _testProjections.dimensions();
    QVERIFY(projDatIO->write(_testProjections, fileName));

    auto metaInfo = projDatIO->metaInfo(fileName);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimChans).toUInt(), dims.nbChannels);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimRows).toUInt(), dims.nbRows);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimZ).toUInt(), dims.nbModules * _testProjections.nbViews());
    QCOMPARE(metaInfo.value(CTL::io::meta_info::typeHint).toString(), CTL::io::meta_info::type_hint::projection);

    auto r = projDatIO->readSingleView(fileName, 1, dims.nbModules);
    QCOMPARE(r.module(0)(3,2), _testProjections.view(1).module(0)(3,2));

    // single view
    auto singleViewProj = _testProjections.view(0);
    QVERIFY(projDatIO->write(singleViewProj, fileName));

    metaInfo = projDatIO->metaInfo(fileName);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimChans).toUInt(), dims.nbChannels);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimRows).toUInt(), dims.nbRows);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimZ).toUInt(), dims.nbModules);
    r = projDatIO->readSingleView(fileName, 0, dims.nbModules);
    QCOMPARE(r.module(0)(3,2), _testProjections.view(0).module(0)(3,2));
}

void DenFileIOtest::processAbstractProjMat(io::AbstractProjectionMatrixIO* projMatIO, const QString &fileName)
{
    QVERIFY(projMatIO->write(_testGeometry, fileName));

    auto metaInfo = projMatIO->metaInfo(fileName);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimX).toInt(), 4);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimY).toInt(), 3);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimZ).toInt(), _testGeometry.at(0).length() * _testGeometry.size());
    QCOMPARE(metaInfo.value(CTL::io::meta_info::typeHint).toString(), CTL::io::meta_info::type_hint::projMatrix);

    auto r = projMatIO->readSingleViewGeometry(fileName, 2, _testGeometry.at(0).length());
    auto valRead = r.at(0).get<1,2>();
    auto valOrig = _testGeometry.at(2).at(0).get<1,2>();
    QCOMPARE(valRead, valOrig);

    // single view
    auto singleViewGeo = _testGeometry.at(1);
    QVERIFY(projMatIO->write(singleViewGeo, fileName));

    metaInfo = projMatIO->metaInfo(fileName);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimX).toInt(), 4);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimY).toInt(), 3);
    QCOMPARE(metaInfo.value(CTL::io::meta_info::dimZ).toInt(), _testGeometry.at(1).length());
    QCOMPARE(metaInfo.value(CTL::io::meta_info::typeHint).toString(), CTL::io::meta_info::type_hint::projMatrix);

    r = projMatIO->readSingleViewGeometry(fileName, 0, _testGeometry.at(0).length());
    valRead = r.at(1).get<2,3>();
    valOrig = _testGeometry.at(1).at(1).get<2,3>();
    QCOMPARE(valRead, valOrig);
}
