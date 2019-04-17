#include "ctldatabase.h"
#include <QCoreApplication>

namespace CTL {

CTLDatabaseHandler::CTLDatabaseHandler()
{
    auto fileWithDatabasePath = []
    {
        QString ret = QCoreApplication::applicationDirPath();
        if(!ret.isEmpty() &&
           !ret.endsWith(QStringLiteral("/")) &&
           !ret.endsWith(QStringLiteral("\\")))
            ret += QStringLiteral("/");
        ret += QStringLiteral("database.path");
        return ret;
    };

    QFile file(fileWithDatabasePath());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "cannot open 'database.path'";
        return;
    }

    auto dbRoot = file.readLine();
    dbRoot.chop(1);
    this->setDataBaseRoot(dbRoot);
}

CTLDatabaseHandler& CTLDatabaseHandler::instance()
{
    static CTLDatabaseHandler theInstance;
    return theInstance;
}

void CTLDatabaseHandler::setDataBaseRoot(const QString &path)
{
    if(!QFile::exists(path))
        qWarning() << "Directory " << path << " does not exist.";
    _dbRoot.setPath(path);

    makeFileMap();
}

std::shared_ptr<AbstractIntegrableDataModel> CTLDatabaseHandler::loadAttenuationModel(database::composite composite)
{
    return _serializer.deserialize<AbstractIntegrableDataModel>(_fileMap.value(int(composite)));
}

std::shared_ptr<AbstractIntegrableDataModel> CTLDatabaseHandler::loadAttenuationModel(database::element element)
{
    return _serializer.deserialize<AbstractIntegrableDataModel>(_fileMap.value(int(element)));
}

std::shared_ptr<TabulatedDataModel> CTLDatabaseHandler::loadXRaySpectrum(database::spectrum spectrum)
{
    return _serializer.deserialize<TabulatedDataModel>(_fileMap.value(int(spectrum)));
}

void CTLDatabaseHandler::makeFileMap()
{
    static QStringList elementFiles{
             "z01",
             "z02",
             "z03",
             "z04",
             "z05",
             "z06",
             "z07",
             "z08",
             "z09",
             "z10",
             "z11",
             "z12",
             "z13",
             "z14",
             "z15",
             "z16",
             "z17",
             "z18",
             "z19",
             "z20",
             "z21",
             "z22",
             "z23",
             "z24",
             "z25",
             "z26",
             "z27",
             "z28",
             "z29",
             "z30",
             "z31",
             "z32",
             "z33",
             "z34",
             "z35",
             "z36",
             "z37",
             "z38",
             "z39",
             "z40",
             "z41",
             "z42",
             "z43",
             "z44",
             "z45",
             "z46",
             "z47",
             "z48",
             "z49",
             "z50",
             "z51",
             "z52",
             "z53",
             "z54",
             "z55",
             "z56",
             "z57",
             "z58",
             "z59",
             "z60",
             "z61",
             "z62",
             "z63",
             "z64",
             "z65",
             "z66",
             "z67",
             "z68",
             "z69",
             "z70",
             "z71",
             "z72",
             "z73",
             "z74",
             "z75",
             "z76",
             "z77",
             "z78",
             "z79",
             "z80",
             "z81",
             "z82",
             "z83",
             "z84",
             "z85",
             "z86",
             "z87",
             "z88",
             "z89",
             "z90",
             "z91",
             "z92"
    };
    static QStringList compositeFiles{
        "adipose",
        "air",
        "c552",
        "alanine",
        "ceric",
        "bakelite",
        "blood",
        "bone",
        "b100",
        "brain",
        "breast",
        "telluride",
        "fluoride",
        "calcium",
        "cesium",
        "concreteba",
        "concrete",
        "eye",
        "fricke",
        "gadolinium",
        "gafchromic",
        "gallium",
        "pyrex",
        "glass",
        "lithiumflu",
        "lithium",
        "lung",
        "magnesium",
        "mercuric",
        "muscle",
        "ovary",
        "kodak",
        "photoemul",
        "vinyl",
        "polyethylene",
        "mylar",
        "pmma",
        "polystyrene",
        "polyvinyl",
        "nylonfilm",
        "teflon",
        "testis",
        "temethane",
        "tepropane",
        "a150",
        "tissue",
        "tissue4",
        "water"
    };
    static QStringList spectraFiles{
        "120kVp_30deg_1000Air"
    };

    for(int el = 0; el < elementFiles.size(); ++el)
    {
        if(!QFile::exists(_dbRoot.absolutePath() + "/attenuation_spectra/" + elementFiles.at(el) + ".json"))
            qWarning() << "Database is missing expected file: " << elementFiles.at(el) + ".json";
        _fileMap.insert(el+1, _dbRoot.absolutePath() + "/attenuation_spectra/" + elementFiles.at(el) + ".json");
    }

    for(int comp = 0; comp < compositeFiles.size(); ++comp)
    {
        if(!QFile::exists(_dbRoot.absolutePath() + "/attenuation_spectra/" + compositeFiles.at(comp) + ".json"))
            qWarning() << "Database is missing expected file: " << compositeFiles.at(comp) + ".json";
        _fileMap.insert(comp+1001, _dbRoot.absolutePath() + "/attenuation_spectra/" + compositeFiles.at(comp) + ".json");
    }

    for(int spec = 0; spec < spectraFiles.size(); ++spec)
    {
        if(!QFile::exists(_dbRoot.absolutePath() + "/xray_spectra/" + spectraFiles.at(spec) + ".json"))
            qWarning() << "Database is missing expected file: " << spectraFiles.at(spec) + ".json";
        _fileMap.insert(spec+2001, _dbRoot.absolutePath() + "/xray_spectra/" + spectraFiles.at(spec) + ".json");
    }
}

std::shared_ptr<AbstractIntegrableDataModel> database::attenuationModel(database::element element)
{
    return CTLDatabaseHandler::instance().loadAttenuationModel(element);
}

std::shared_ptr<AbstractIntegrableDataModel> database::attenuationModel(database::composite composite)
{
    return CTLDatabaseHandler::instance().loadAttenuationModel(composite);
}

std::shared_ptr<TabulatedDataModel> database::xRaySpectrum(database::spectrum spectrum)
{
    return CTLDatabaseHandler::instance().loadXRaySpectrum(spectrum);
}

}
