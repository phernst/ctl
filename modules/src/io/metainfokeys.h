#ifndef METAINFOKEYS_H
#define METAINFOKEYS_H

#include <QMetaType>
#include <QString>

/*
 * NOTE: This is header only.
 */

// Standardized tags (keys and values) for meta information of files
// containing images (or matrices) for CTL base types:
//   - volume slices
//   - projection images
//   - projection matrices

namespace CTL {
namespace io {
namespace meta_info {

// # basic data dimensions (apply to all types)
struct Dimensions; // value type
const QString dimensions = QStringLiteral("dimensions"); // key

// # dimension interpretation
const QString dim1Type = QStringLiteral("dimension 1 type"); // key
const QString dim2Type = QStringLiteral("dimension 2 type"); // key
const QString dim3Type = QStringLiteral("dimension 3 type"); // key
const QString dim4Type = QStringLiteral("dimension 4 type"); // key

// volume
const QString nbVoxelsX = QStringLiteral("num vox x"); // value
const QString nbVoxelsY = QStringLiteral("num vox y"); // value
const QString nbVoxelsZ = QStringLiteral("num vox z"); // value

// projection data/matrices
const QString nbRows = QStringLiteral("num row");        // value
const QString nbChans = QStringLiteral("num channel");   // value, proj. data
const QString nbCols = QStringLiteral("num column");     // value, proj. matrices
const QString nbViews = QStringLiteral("num proj");      // value
const QString nbMods = QStringLiteral("num det module"); // value

// # additional volume info
const QString voxSizeX = QStringLiteral("vox size x"); // key
const QString voxSizeY = QStringLiteral("vox size y"); // key
const QString voxSizeZ = QStringLiteral("vox size z"); // key

const QString volOffX = QStringLiteral("recon center x"); // key
const QString volOffY = QStringLiteral("recon center y"); // key
const QString volOffZ = QStringLiteral("recon center z"); // key

// # type info
const QString typeHint = QStringLiteral("type hint"); // key
namespace type_hint {
    const QString projection = QStringLiteral("projection data");   // value
    const QString projMatrix = QStringLiteral("projection matrix"); // value
    const QString volume = QStringLiteral("volume data");           // value
    const QString slice = QStringLiteral("slice");                  // value
} // namespace type_hint

struct Dimensions
{
    Dimensions()
        : nbDim(0), dim1(0), dim2(0), dim3(0), dim4(0)
    {}
    Dimensions(uint dim1, uint dim2)
        : nbDim(2), dim1(dim1), dim2(dim2), dim3(0), dim4(0)
    {}
    Dimensions(uint dim1, uint dim2, uint dim3)
        : nbDim(3), dim1(dim1), dim2(dim2), dim3(dim3), dim4(0)
    {}
    Dimensions(uint dim1, uint dim2, uint dim3, uint dim4)
        : nbDim(4), dim1(dim1), dim2(dim2), dim3(dim3), dim4(dim4)
    {}

    const uint nbDim;
    const uint dim1;
    const uint dim2;
    const uint dim3;
    const uint dim4;
};

} // namespace meta_info
} // namespace io
} // namespace CTL

Q_DECLARE_METATYPE(CTL::io::meta_info::Dimensions)

#endif // METAINFOKEYS_H
