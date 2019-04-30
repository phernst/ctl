#ifndef NRRDFILEIO_H
#define NRRDFILEIO_H

#include "io/basetypeio.h"
#include <QFile>
#include <QRegularExpression>

/*
 * NOTE: This is header only.
 */

namespace CTL {
namespace io {

class NrrdFileIO
{
public:
    enum DataType { Char, UChar, Short, UShort, Int, UInt, Int64, UInt64, Float, Double, Block };

    QVariantMap metaInfo(const QString& fileName) const;

    template <typename T>
    std::vector<T> readAll(const QString& fileName) const;
    template <typename T>
    std::vector<T> readChunk(const QString& fileName, uint chunkNb) const;

    template <typename T>
    bool
    write(const std::vector<T>& data, const QVariantMap& metaInfo, const QString& fileName) const;

    void setSkipComments(bool skipComments);
    void setSkipKeyValuePairs(bool skipKeyValuePairs);
    bool skipComments() const;
    bool skipKeyValuePairs() const;

private:

    bool _skipComments = true;
    bool _skipKeyValuePairs = false;

    template <typename T>
    bool checkHeader(const QVariantMap& metaInfo);
    template <typename T>
    static DataType dataType();
    static DataType dataTypeFromString(const QString& typeString);
    static bool isBigEndian();
    bool parseField(const QString& field, const QString& desc,
                    QVariantMap* metaInfo, int* nbDimension) const;
    static int sizeOfType(DataType type);
};

} // namespace io
} // namespace CTL

#include "nrrdfileio.tpp"

#endif // NRRDFILEIO_H
