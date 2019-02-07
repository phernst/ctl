#ifndef METAINFOKEYS_H
#define METAINFOKEYS_H

#include <QString>

namespace CTL {
namespace io {
namespace meta_info {

const QString dimensions = QStringLiteral("dimensions");

const QString dim1Type = QStringLiteral("dimension 1 type");
const QString dim2Type = QStringLiteral("dimension 2 type");
const QString dim3Type = QStringLiteral("dimension 3 type");
const QString dim4Type = QStringLiteral("dimension 4 type");

const QString nbVoxelsX = QStringLiteral("num vox x");
const QString nbVoxelsY = QStringLiteral("num vox y");
const QString nbVoxelsZ = QStringLiteral("num vox z");

const QString nbRows = QStringLiteral("num row");
const QString nbChans = QStringLiteral("num channel");
const QString nbCols = QStringLiteral("num column");
const QString nbViews = QStringLiteral("num proj");
const QString nbMods = QStringLiteral("num det module");

const QString voxSizeX = QStringLiteral("vox size x");
const QString voxSizeY = QStringLiteral("vox size y");
const QString voxSizeZ = QStringLiteral("vox size z");

const QString volOffX = QStringLiteral("recon center x");
const QString volOffY = QStringLiteral("recon center y");
const QString volOffZ = QStringLiteral("recon center z");

const QString typeHint = QStringLiteral("type hint");
namespace type_hint {
    const QString projection = QStringLiteral("projection data");
    const QString projMatrix = QStringLiteral("projection matrix");
    const QString volume = QStringLiteral("volume data");
    const QString slice = QStringLiteral("slice");
} // namespace type_hint

} // namespace meta_info
} // namespace io
} // namespace CTL

#endif // METAINFOKEYS_H
