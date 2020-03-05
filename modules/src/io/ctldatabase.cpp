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

void CTLDatabaseHandler::setDataBaseRoot(const QString& path)
{
    if(!QFile::exists(path))
        qWarning() << "Directory " << path << " does not exist.";
    _dbRoot.setPath(path);

    makeFileMap();
}

std::shared_ptr<AbstractIntegrableDataModel>
CTLDatabaseHandler::loadAttenuationModel(database::Composite composite)
{
    return _serializer.deserialize<AbstractIntegrableDataModel>(_fileMap.value(int(composite)));
}

std::shared_ptr<AbstractIntegrableDataModel>
CTLDatabaseHandler::loadAttenuationModel(database::Element element)
{
    return _serializer.deserialize<AbstractIntegrableDataModel>(_fileMap.value(int(element)));
}

std::shared_ptr<TabulatedDataModel>
CTLDatabaseHandler::loadXRaySpectrum(database::Spectrum spectrum)
{
    return _serializer.deserialize<TabulatedDataModel>(_fileMap.value(int(spectrum)));
}

float CTLDatabaseHandler::loadDensity(database::Composite composite)
{
    return JsonSerializer::variantFromJsonFile(_fileMap.value(int(composite)))
        .toMap()
        .value("density", -1.0f)
        .toFloat();
}

float CTLDatabaseHandler::loadDensity(database::Element element)
{
    return JsonSerializer::variantFromJsonFile(_fileMap.value(int(element)))
        .toMap()
        .value("density", -1.0f)
        .toFloat();
}

void CTLDatabaseHandler::makeFileMap()
{
    static const QStringList elementFiles{
        "z01", "z02", "z03", "z04", "z05", "z06", "z07", "z08", "z09", "z10", "z11", "z12",
        "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24",
        "z25", "z26", "z27", "z28", "z29", "z30", "z31", "z32", "z33", "z34", "z35", "z36",
        "z37", "z38", "z39", "z40", "z41", "z42", "z43", "z44", "z45", "z46", "z47", "z48",
        "z49", "z50", "z51", "z52", "z53", "z54", "z55", "z56", "z57", "z58", "z59", "z60",
        "z61", "z62", "z63", "z64", "z65", "z66", "z67", "z68", "z69", "z70", "z71", "z72",
        "z73", "z74", "z75", "z76", "z77", "z78", "z79", "z80", "z81", "z82", "z83", "z84",
        "z85", "z86", "z87", "z88", "z89", "z90", "z91", "z92"
    };
    static const QStringList compositeFiles{
        "adipose",    "air",         "c552",       "alanine",    "ceric",        "bakelite",
        "blood",      "bone",        "b100",       "brain",      "breast",       "telluride",
        "fluoride",   "calcium",     "cesium",     "concreteba", "concrete",     "eye",
        "fricke",     "gadolinium",  "gafchromic", "gallium",    "pyrex",        "glass",
        "lithiumflu", "lithium",     "lung",       "magnesium",  "mercuric",     "muscle",
        "ovary",      "kodak",       "photoemul",  "vinyl",      "polyethylene", "mylar",
        "pmma",       "polystyrene", "polyvinyl",  "nylonfilm",  "teflon",       "testis",
        "temethane",  "tepropane",   "a150",       "tissue",     "tissue4",      "water"
    };
    static const QStringList xraySpectraFiles{
        "120kVp_30deg_1000Air"
    };

    QString folder;
    int mapPos;

    auto insertInFileMap = [&](const QString& fn) {
        const auto path = folder + fn + ".json";
        if(!QFile::exists(path))
            qWarning() << "Database is missing expected file:\n" + path;

        _fileMap.insert(mapPos++, path);
    };

    folder = _dbRoot.absolutePath() + "/attenuation_spectra/";
    mapPos = 1;
    std::for_each(elementFiles.constBegin(), elementFiles.constEnd(), insertInFileMap);

    mapPos = 1001;
    std::for_each(compositeFiles.constBegin(), compositeFiles.constEnd(), insertInFileMap);

    folder = _dbRoot.absolutePath() + "/xray_spectra/";
    mapPos = 2001;
    std::for_each(xraySpectraFiles.constBegin(), xraySpectraFiles.constEnd(), insertInFileMap);
}

namespace database {

std::shared_ptr<AbstractIntegrableDataModel> attenuationModel(database::Element element)
{
    return CTLDatabaseHandler::instance().loadAttenuationModel(element);
}

std::shared_ptr<AbstractIntegrableDataModel> attenuationModel(database::Composite composite)
{
    return CTLDatabaseHandler::instance().loadAttenuationModel(composite);
}

std::shared_ptr<TabulatedDataModel> xRaySpectrum(database::Spectrum spectrum)
{
    return CTLDatabaseHandler::instance().loadXRaySpectrum(spectrum);
}

float density(database::Composite composite)
{
    return CTLDatabaseHandler::instance().loadDensity(composite);
}

float density(database::Element element)
{
    return CTLDatabaseHandler::instance().loadDensity(element);
}

} // namespace database
} // namespace CTL
