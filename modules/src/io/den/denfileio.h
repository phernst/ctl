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

    ret.insert(meta_info::dimX, QVariant(header.cols));
    ret.insert(meta_info::dimY, QVariant(header.rows));
    ret.insert(meta_info::dimZ, QVariant(header.count));

    ret.insert(meta_info::dimChans, QVariant(header.cols));
    ret.insert(meta_info::dimRows, QVariant(header.rows));

    return ret;
}

template<typename T>
bool DenFileIO::write(const std::vector<T> &data,
                      const QVariantMap &metaInfo,
                      const QString &fileName) const
{
    den::DFile dFile(fileName);
    dFile.setVerbose(false);

    den::Header header;
    if(metaInfo.contains(meta_info::dimX) &&
       metaInfo.contains(meta_info::dimY) &&
       metaInfo.contains(meta_info::dimZ))
    {
        header.cols  = metaInfo.value(meta_info::dimX).toInt();
        header.rows  = metaInfo.value(meta_info::dimY).toInt();
        header.count = metaInfo.value(meta_info::dimZ).toInt();
    }
    else if (metaInfo.contains(meta_info::dimChans) &&
             metaInfo.contains(meta_info::dimRows) &&
             metaInfo.contains(meta_info::dimMods) &&
             metaInfo.contains(meta_info::dimViews))
    {
        header.cols  = metaInfo.value(meta_info::dimChans).toInt();
        header.rows  = metaInfo.value(meta_info::dimRows).toInt();
        header.count = metaInfo.value(meta_info::dimMods).toInt() *
                       metaInfo.value(meta_info::dimViews).toInt();
    }
    else
        throw std::runtime_error("Writing aborted: missing data meta information!");

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
