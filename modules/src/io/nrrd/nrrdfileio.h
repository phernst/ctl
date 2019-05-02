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
    // supported raw/binary data types
    enum DataType { Char, UChar, Short, UShort, Int, UInt, Int64, UInt64, Float, Double, Block };

    // implementer interface
    QVariantMap metaInfo(const QString& fileName) const;

    template <typename T>
    std::vector<T> readAll(const QString& fileName) const;
    template <typename T>
    std::vector<T> readChunk(const QString& fileName, uint chunkNb) const;

    template <typename T>
    bool
    write(const std::vector<T>& data, const QVariantMap& metaInfo, const QString& fileName) const;

    // specific configuration
    void setSkipComments(bool skipComments);
    void setSkipKeyValuePairs(bool skipKeyValuePairs);
    bool skipComments() const;
    bool skipKeyValuePairs() const;

private:

    bool _skipComments = true;
    bool _skipKeyValuePairs = false;

    template <typename T>
    bool checkHeader(const QVariantMap& metaInfo) const;
    template <typename T>
    bool writeHeader(std::ofstream& file, const QVariantMap& metaInfo) const;
    template <typename T>
    static DataType dataType();
    static DataType dataTypeFromString(const QString& typeString);
    static bool isBigEndian();
    bool parseField(const QString& field, const QString& desc,
                    QVariantMap* metaInfo, int* nbDimension) const;
    static int sizeOfType(DataType type);
    static const char* stringOfType(DataType type);

    // Nrrd fields which are translated to basetype meta info
    const QString _fDimension = QStringLiteral("dimension");
    const QString _fEncoding = QStringLiteral("encoding");
    const QString _fEndianness = QStringLiteral("endian");
    const QString _fLabels = QStringLiteral("labels");
    const QString _fSizes = QStringLiteral("sizes");
    const QString _fSpaceOrigin = QStringLiteral("space origin");
    const QString _fSpacings = QStringLiteral("spacings");
    const QString _fType = QStringLiteral("type");
};

} // namespace io
} // namespace CTL

#include "nrrdfileio.tpp"

#endif // NRRDFILEIO_H
