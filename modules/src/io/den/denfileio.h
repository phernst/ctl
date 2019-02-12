#ifndef DENFILEIO_H
#define DENFILEIO_H

#include "dfileformat.h"

#ifndef CTL_CORE_MODULE_NOT_AVAILABLE

#include "io/basetypeio.h"
#include "io/metainfokeys.h"
#include <QVariantMap>

/*
 * NOTE: This is header only.
 */

namespace CTL {
namespace io {

class DenFileIO
{
public:
    QVariantMap metaInfo(const QString& fileName) const;

    template <typename T>
    std::vector<T> readAll(const QString& fileName) const;
    template <typename T>
    std::vector<T> readChunk(const QString& fileName, uint chunkNb) const;

    template <typename T>
    bool write(const std::vector<T>& data,
               const QVariantMap& metaInfo,
               const QString& fileName) const;
};

// ######## implementation #########

inline QVariantMap DenFileIO::metaInfo(const QString &fileName) const
{
    QVariantMap ret;

    auto header = den::loadHeader(fileName);

    meta_info::Dimensions dimensions( header.cols,
                                      header.rows,
                                      header.count );

    ret.insert(meta_info::dimensions, QVariant::fromValue(dimensions));

    return ret;
}

template<typename T>
bool DenFileIO::write(const std::vector<T> &data,
                      const QVariantMap &metaInfo,
                      const QString &fileName) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    auto dimList = metaInfo.value(meta_info::dimensions).value<meta_info::Dimensions>();

    if(dimList.nbDim < 2)
        throw std::runtime_error("Writing aborted: missing data meta information!");

    den::Header header;
    header.cols = dimList.dim0;
    header.rows = dimList.dim1;
    header.count = (dimList.dim2 ? dimList.dim2 : 1) * (dimList.dim3 ? dimList.dim3 : 1);

    return dFile.save(data, header);
}

template <>
inline std::vector<uchar> DenFileIO::readChunk(const QString& fileName, uint chunkNb) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadUChar(chunkNb, 1);
}

template <>
inline std::vector<ushort> DenFileIO::readChunk(const QString& fileName, uint chunkNb) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadUShort(chunkNb, 1);
}

template <>
inline std::vector<float> DenFileIO::readChunk(const QString& fileName, uint chunkNb) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadFloat(chunkNb, 1);
}

template <>
inline std::vector<double> DenFileIO::readChunk(const QString& fileName, uint chunkNb) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadDouble(chunkNb, 1);
}

template <>
inline std::vector<uchar> DenFileIO::readAll(const QString& fileName) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadUChar();
}

template <>
inline std::vector<ushort> DenFileIO::readAll(const QString& fileName) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadUShort();
}

template <>
inline std::vector<float> DenFileIO::readAll(const QString& fileName) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadFloat();
}

template <>
inline std::vector<double> DenFileIO::readAll(const QString& fileName) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    return dFile.loadDouble();
}

} // namespace io
} // namespace CTL

#endif // CTL_CORE_MODULE_NOT_AVAILABLE

#endif // DENFILEIO_H
