#ifndef METAINFOKEYS_H
#define METAINFOKEYS_H

#include <QString>

namespace CTL {
namespace io {

namespace meta_info {

const QString dimX = QStringLiteral("size 1");
const QString dimY = QStringLiteral("size 2");
const QString dimZ = QStringLiteral("size 3");

const QString dimRows = QStringLiteral("num row");
const QString dimChans = QStringLiteral("num channels");
const QString dimViews = QStringLiteral("num proj");
const QString dimMods = QStringLiteral("num det module");

const QString voxSizeX = QStringLiteral("vox size x");
const QString voxSizeY = QStringLiteral("vox size y");
const QString voxSizeZ = QStringLiteral("vox size z");

const QString volOffX = QStringLiteral("recon center x");
const QString volOffY = QStringLiteral("recon center y");
const QString volOffZ = QStringLiteral("recon center z");

const QString typeHint = QStringLiteral("type hint");
namespace type_hint {
    const QString volume = QStringLiteral("volume data");
    const QString projection = QStringLiteral("projection data");
    const QString projMatrix = QStringLiteral("projection matrix");
}

}

}
}

#endif // METAINFOKEYS_H
