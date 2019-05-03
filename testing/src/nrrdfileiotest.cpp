#include "nrrdfileiotest.h"
#include "io/nrrd/nrrdfileio.h"

typedef CTL::io::BaseTypeIO<CTL::io::NrrdFileIO> IOtype;

const QString headerOnlyFile = QStringLiteral("testData/header.nrrd");

/*
 * NRRD0004
type: float
dimension: 3
sizes: 256 256 199
encoding: raw
endian: little
# asd1
#asd2
##asd3
myKey:=myValue
*/

void NrrdFileIOtest::testMetaInfo()
{
    auto metaInfoReader = IOtype::makeMetaInfoReader();
    auto metaInfo = metaInfoReader->metaInfo(headerOnlyFile);

    auto dims = metaInfo.value(CTL::io::meta_info::dimensions).value<CTL::io::meta_info::Dimensions>();
    QCOMPARE(dims.nbDim, 3u);
    QCOMPARE(dims.dim1, 256u);
    QCOMPARE(dims.dim2, 256u);
    QCOMPARE(dims.dim3, 199u);
}

void NrrdFileIOtest::testFields()
{
    auto metaInfoReader = IOtype::makeMetaInfoReader();
    auto metaInfo = metaInfoReader->metaInfo(headerOnlyFile);

    QString fieldName;
    fieldName = metaInfo.value("encoding").toString();
    QCOMPARE(fieldName, QStringLiteral("raw"));
    fieldName = metaInfo.value("endian").toString();
    QCOMPARE(fieldName, QStringLiteral("little"));

    fieldName = metaInfo.value("type").toString();
    QCOMPARE(fieldName, QStringLiteral("float"));
    QCOMPARE(metaInfo.value("data type enum").toInt(), int(CTL::io::NrrdFileIO::DataType::Float));
}

void NrrdFileIOtest::testHeaderProperties()
{
    auto metaInfoReader = IOtype::makeMetaInfoReader();
    auto metaInfo = metaInfoReader->metaInfo(headerOnlyFile);

    auto version = metaInfo.value("nrrd version").toInt();
    QCOMPARE(version, 4);

    auto headerSize = metaInfo.value("nrrd header offset").toInt();
    QCOMPARE(headerSize, 118);
}

/*
void NrrdFileIOtest::initTestCase()
{
    using namespace CTL;
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
*/


