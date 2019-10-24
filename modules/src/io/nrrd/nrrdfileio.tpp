#include "nrrdfileio.h"
#include <fstream>

// checks format of floating point numbers at compile time and leads to a compiler
// error if the platform does not support floating point numbers according to IEEE 754
static_assert(std::numeric_limits<float>::is_iec559, "'float' must be compliant with the IEEE 754 standard");
static_assert(std::numeric_limits<double>::is_iec559, "'double' must be compliant with the IEEE 754 standard");
static_assert(sizeof(float)  * CHAR_BIT == 32, "'float' must be 32 Bit");
static_assert(sizeof(double) * CHAR_BIT == 64, "'double' must be 64 Bit");
// checks sizes of integer formats
static_assert(sizeof(char)    * CHAR_BIT == 8,  "'char' must be 8 Bit");
static_assert(sizeof(short)   * CHAR_BIT == 16, "'short' must be 16 Bit");
static_assert(sizeof(int)     * CHAR_BIT == 32, "'int' must be 32 Bit");
static_assert(sizeof(int64_t) * CHAR_BIT == 64, "'int64' must be 64 Bit");

namespace CTL {
namespace io {

inline QVariantMap NrrdFileIO::metaInfo(const QString& fileName) const
{
    QVariantMap ret;
    static const QRegularExpression readComment("^#[# ]*(?<commentString>.*)");
    static const QRegularExpression skipComment("^#");
    static const QRegularExpression field("^(?<field>.+): (?<desc>\\s*?\\S*(:?\\s+\\S+)*)\\s*$");
    static const QRegularExpression keyValuePair("^(?<key>.+?):=(?<value>.*)$");
    static const QRegularExpression skipKeyValuePair("^.+?:=");
    static const QRegularExpression nrrdMagic("^NRRD000(?<version>\\d)$");

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "cannot open file:" << fileName;
        return ret;
    }

    // first line
    auto firstLine = file.readLine(12);
    auto mFirstLine = nrrdMagic.match(firstLine);
    if(!mFirstLine.hasMatch())
    {
        qCritical() << "no valid nrrd file - the magic first line is missing:" << fileName;
        return ret;
    }
    ret.insert("nrrd version", mFirstLine.captured("version").toInt());

    // rest of header
    int commentCounter = 0; // only for comments (if _skipComments == false)
    int dimension = 0;
    while(!file.atEnd())
    {
        auto line = file.readLine();
        // comment capture or skip mode
        auto mComment = _skipComments ? skipComment.match(line)
                                      : readComment.match(line);

        // 1. check if comment
        if(mComment.hasMatch())
        {
            if(!_skipComments)
                ret.insert(QStringLiteral("comment ") + QString::number(commentCounter++),
                           mComment.captured("commentString"));
        }
        else
        {
            // key-value pair capture or skip mode
            auto mKeyValuePair = _skipKeyValuePairs ? skipKeyValuePair.match(line)
                                                    : keyValuePair.match(line);
            // 2. check if key-value pair
            if(mKeyValuePair.hasMatch())
            {
                if(!_skipKeyValuePairs)
                    ret.insert(mKeyValuePair.captured("key"), mKeyValuePair.captured("value"));
            }
            else
            {
                auto mField = field.match(line);
                // 3. check if field
                if(mField.hasMatch())
                {
                    if(!parseField(mField.captured("field"), mField.captured("desc"), &ret, &dimension))
                    {
                        qCritical() << "invalid field entry:" << mField.captured("field")
                                    << mField.captured("desc") << "in file" << fileName;
                        return ret;
                    }
                }
                else
                {   // 4. check if empty line (end of header)
                    if(line == "\n")
                    {
                        // end of header
                        break;
                    }
                    else
                    {
                        // 5. invalid line
                        qCritical() << "invalid header entry:" << line << "\nin file" << fileName;
                        return ret;
                    }
                }
            }
        }
    }
    ret.insert("nrrd header offset", file.pos());
    file.close();
    return ret;
}

template <typename T>
std::vector<T> NrrdFileIO::readAll(const QString& fileName) const
{
    std::vector<T> ret;

    NrrdFileIO io;
    io.setSkipComments(true);
    io.setSkipKeyValuePairs(true);
    auto metaInfo = io.metaInfo(fileName);

    if(!checkHeader<T>(metaInfo))
        return ret;

    const auto headerOffset = metaInfo.value("nrrd header offset").toLongLong();
    const auto dimensions = metaInfo.value(meta_info::dimensions).value<meta_info::Dimensions>();

    std::ifstream file(fileName.toStdString(), std::ios::binary | std::ios::ate);
    if(!file)
    {
        qCritical() << "unable to open file" << fileName;
        return ret;
    }

    // get length data block
    const int64_t bytesOfFile = file.tellg();
    const int64_t dataBytes = bytesOfFile - headerOffset;
    const int64_t nbElements = dimensions.dim1 * dimensions.dim2 *
            (dimensions.nbDim >= 3 ? dimensions.dim3 : 1) *
            (dimensions.nbDim == 4 ? dimensions.dim4 : 1);

    if(nbElements * int64_t(sizeof(T)) != dataBytes)
    {
        qCritical() << "raw data size of file does not fit to dimensions in nrrd header";
        return ret;
    }

    ret.resize(nbElements);
    file.seekg(headerOffset);
    file.read(reinterpret_cast<char*>(ret.data()), dataBytes);

    if(!file)
        qCritical() << "only" << file.gcount() << "could be read";

    file.close();
    return ret;
}

template<typename T>
std::vector<T> NrrdFileIO::readChunk(const QString &fileName, uint chunkNb) const
{
    std::vector<T> ret;

    NrrdFileIO io;
    io.setSkipComments(true);
    io.setSkipKeyValuePairs(true);
    auto metaInfo = io.metaInfo(fileName);

    if(!checkHeader<T>(metaInfo))
        return ret;

    const auto headerOffset = metaInfo.value("nrrd header offset").toLongLong();
    const auto dimensions = metaInfo.value(meta_info::dimensions).value<meta_info::Dimensions>();

    size_t nbChunks;
    switch (dimensions.nbDim) {
    case 2: nbChunks = 1;
        break;
    case 3: nbChunks = dimensions.dim3;
        break;
    case 4: nbChunks = dimensions.dim3 * dimensions.dim4;
        break;
    default: qCritical("invalid number of dimensions");
        return ret;
    }
    if(chunkNb >= nbChunks)
    {
        qCritical() << "chunk exceeds total number of chunks in file: "
                    << chunkNb << '/' << nbChunks;
        return ret;
    }

    std::ifstream file(fileName.toStdString(), std::ios::binary | std::ios::ate);
    if(!file)
    {
        qCritical() << "unable to open file" << fileName;
        return ret;
    }

    // get length data block
    const int64_t bytesOfFile = file.tellg();
    const int64_t dataBytes = bytesOfFile - headerOffset;
    const size_t nbElements = dimensions.dim1 * dimensions.dim2;
    const size_t bytes2read = nbElements * sizeof(T);

    if(nbElements * nbChunks * sizeof(T) != size_t(dataBytes))
    {
        qCritical() << "raw data size of file does not fit to dimensions in nrrd header";
        return ret;
    }

    ret.resize(nbElements);
    file.seekg(headerOffset + chunkNb * bytes2read);
    file.read(reinterpret_cast<char*>(ret.data()), bytes2read);

    if(!file)
        qCritical() << "only" << file.gcount() << "could be read";

    file.close();
    return ret;
}

template <typename T>
bool NrrdFileIO::write(const std::vector<T>& data,
                       const QVariantMap& metaInfo,
                       const QString& fileName) const
{
    // open file
    std::ofstream file(fileName.toStdString(), std::ios::binary);
    if(!file.is_open())
    {
        qCritical("cannot open file");
        return false;
    }

    // header
    if(!writeHeader<T>(file, metaInfo))
        return false;

    // binary data
    auto bytes2write = data.size() * sizeof(T);
    file.write(reinterpret_cast<const char*>(data.data()), bytes2write);

    file.close();
    if(!file)
    {
        qCritical("writing to file failed");
        return false;
    }

    return true;
}

template<typename T>
bool NrrdFileIO::writeHeader(std::ofstream& file, const QVariantMap& metaInfo) const
{
    auto type = dataType<T>();
    auto dims = metaInfo.value(meta_info::dimensions).value<meta_info::Dimensions>();
    QString dimensionTypes;

    // first line
    static const char nrrdVersion[] = "NRRD0004\n";
    file.write(nrrdVersion, sizeof(nrrdVersion)-1);

    // fields
    file << _fType.toStdString() << ": " << stringOfType(type) << '\n';

    file << _fDimension.toStdString() << ": " << dims.nbDim << '\n';

    file << _fSizes.toStdString() << ": ";
    switch (dims.nbDim) {
    case 2: file << dims.dim1 << " " << dims.dim2;
        dimensionTypes = '"' + metaInfo.value(meta_info::dim1Type).toString()
                   + "\" \"" + metaInfo.value(meta_info::dim2Type).toString() + '"';
        break;
    case 3: file << dims.dim1 << " " << dims.dim2 << " " << dims.dim3;
        dimensionTypes = '"' + metaInfo.value(meta_info::dim1Type).toString()
                   + "\" \"" + metaInfo.value(meta_info::dim2Type).toString()
                   + "\" \"" + metaInfo.value(meta_info::dim3Type).toString() + '"';
        break;
    case 4: file << dims.dim1 << " " << dims.dim2 << " " << dims.dim3 << " " << dims.dim4;
        dimensionTypes = '"' + metaInfo.value(meta_info::dim1Type).toString()
                   + "\" \"" + metaInfo.value(meta_info::dim2Type).toString()
                   + "\" \"" + metaInfo.value(meta_info::dim3Type).toString()
                   + "\" \"" + metaInfo.value(meta_info::dim4Type).toString() + '"';
        break;
    default: qCritical("invalid number of dimensions");
        return false;
    }
    file.put('\n');

    file << _fLabels.toStdString() << ": " << dimensionTypes.toStdString() << '\n';

    file << _fEncoding.toStdString() << ": raw\n";

    file << _fEndianness.toStdString() << ": " << (isBigEndian() ? "big" : "little") << '\n';

    if(metaInfo.contains(meta_info::voxSizeX))
    {
        file << _fSpacings.toStdString() << ": "
             << metaInfo.value(meta_info::voxSizeX).toString().toStdString() << ' '
             << metaInfo.value(meta_info::voxSizeY).toString().toStdString() << ' '
             << metaInfo.value(meta_info::voxSizeZ).toString().toStdString() << '\n';

        file << "units:";
        for(uint d = 0; d < dims.nbDim; ++d)
        {
            file << " \"mm\"";
        }
        file.put('\n');
    }

    if(metaInfo.contains(meta_info::volOffX))
        file << "space: scanner-xyz\n"
             << _fSpaceOrigin.toStdString() << ": ("
             << metaInfo.value(meta_info::volOffX).toString().toStdString() << ','
             << metaInfo.value(meta_info::volOffY).toString().toStdString() << ','
             << metaInfo.value(meta_info::volOffZ).toString().toStdString() << ")\n";

    if(type == DataType::Block)
    {
        file << "blocksize: " << sizeof(T);
        file.put('\n');
    }

    // key-value pairs
    auto isField = [](const QString& s) {
        return s == meta_info::dimensions ||
               s == meta_info::dim1Type || s == meta_info::dim2Type || s == meta_info::dim3Type ||
               s == meta_info::dim4Type ||
               s == meta_info::voxSizeX || s == meta_info::voxSizeY || s == meta_info::voxSizeZ ||
               s == meta_info::volOffX || s == meta_info::volOffY || s == meta_info::volOffZ;
    };
    for(auto it = metaInfo.begin(), end = metaInfo.end(); it != end; ++it)
        if(it.value().canConvert<QString>() && !isField(it.key()))
            file << it.key().toStdString() << ":=" << it.value().toString().toStdString() << '\n';

    // end of header
    file.put('\n');

    return true;
}

inline bool NrrdFileIO::skipComments() const { return _skipComments; }

inline void NrrdFileIO::setSkipComments(bool skipComments) { _skipComments = skipComments; }

inline bool NrrdFileIO::skipKeyValuePairs() const { return _skipKeyValuePairs; }

inline void NrrdFileIO::setSkipKeyValuePairs(bool skipKeyValuePairs)
{
    _skipKeyValuePairs = skipKeyValuePairs;
}

inline bool NrrdFileIO::parseField(const QString &field, const QString &desc,
                                   QVariantMap* metaInfo, int* nbDimension) const
{
    if(field.compare(_fDimension, Qt::CaseInsensitive) == 0)
    {
        if(*nbDimension)
            return false;
        *nbDimension = desc.toInt();
        return 0 < *nbDimension && *nbDimension <= 4;
    }
    else if(field.compare(_fSizes, Qt::CaseInsensitive) == 0)
    {
        if(*nbDimension == 0 && metaInfo->contains(meta_info::dimensions))
            return false;
        auto dimList = desc.split(' ');
        if(*nbDimension != dimList.length())
            return false;

        switch(*nbDimension)
        {
        case 2:
            metaInfo->insert(meta_info::dimensions,
                             QVariant::fromValue(meta_info::Dimensions(dimList.at(0).toUInt(),
                                                                       dimList.at(1).toUInt())));
            break;
        case 3:
            metaInfo->insert(meta_info::dimensions,
                             QVariant::fromValue(meta_info::Dimensions(dimList.at(0).toUInt(),
                                                                       dimList.at(1).toUInt(),
                                                                       dimList.at(2).toUInt())));
            break;
        case 4:
            metaInfo->insert(meta_info::dimensions,
                             QVariant::fromValue(meta_info::Dimensions(dimList.at(0).toUInt(),
                                                                       dimList.at(1).toUInt(),
                                                                       dimList.at(2).toUInt(),
                                                                       dimList.at(3).toUInt())));
            break;
        default:
            return false;
        }
    }
    else if(field.compare(_fType, Qt::CaseInsensitive) == 0)
    {
        if(metaInfo->contains(_fType))
            return false;
        auto dataTypeID = dataTypeFromString(desc);
        if(dataTypeID < 0)
            return false;
        metaInfo->insert(_fType, desc);
        metaInfo->insert("data type enum", dataTypeID);
    }
    else if(field.compare(_fEncoding, Qt::CaseInsensitive) == 0)
    {
        if(metaInfo->contains(_fEncoding))
            return false;

        if(desc == "raw")
            metaInfo->insert(_fEncoding, "raw");
        else if(desc == "txt" || desc == "text" || desc == "ascii")
            metaInfo->insert(_fEncoding, "ascii");
    }
    else if(field.compare(_fSpacings, Qt::CaseInsensitive) == 0)
    {
        if(metaInfo->contains(meta_info::voxSizeX))
            return false;

        auto spacings = desc.split(' ');
        metaInfo->insert(meta_info::voxSizeX, spacings.value(0).toFloat());
        metaInfo->insert(meta_info::voxSizeY, spacings.value(1).toFloat());
        metaInfo->insert(meta_info::voxSizeZ, spacings.value(2).toFloat());
    }
    else if(field.compare(_fSpaceOrigin, Qt::CaseInsensitive) == 0)
    {
    //space origin: (0.0,1.0,0.3)
        if(metaInfo->contains(meta_info::volOffX))
            return false;
        if(!desc.startsWith('(') || !desc.endsWith(')'))
            return false;
        // extract part between '(' and ')'
        auto offSet = desc.mid(1);
        offSet.chop(1);
        auto vector = offSet.split(',');
        metaInfo->insert(meta_info::volOffX, vector.value(0).toFloat());
        metaInfo->insert(meta_info::volOffY, vector.value(1).toFloat());
        metaInfo->insert(meta_info::volOffZ, vector.value(2).toFloat());
    }
    else if(field.compare(_fLabels, Qt::CaseInsensitive) == 0)
    {
    //labels: "<label[0]>" "<label[1]>" ... "<label[dim-1]>"
        if(metaInfo->contains(meta_info::dim1Type))
            return false;

        auto dimTypes = desc.split(QStringLiteral(" \""));
        dimTypes.first().remove(0, 1);
        for(auto& s : dimTypes) s.chop(1);
        auto nbTypes = dimTypes.length();
        if(nbTypes > 4)
            return false;

        static const QStringList keys{ meta_info::dim1Type, meta_info::dim2Type,
                                       meta_info::dim3Type, meta_info::dim4Type };
        for(int i = 0; i < nbTypes; ++i)
            metaInfo->insert(keys.at(i), dimTypes.at(i));
    }
    else // other field
    {
        auto keyString = field.toLower();
        if(metaInfo->contains(keyString))
            return false;
        metaInfo->insert(keyString, desc);
    }

    return true;
}

inline NrrdFileIO::DataType NrrdFileIO::dataTypeFromString(const QString &desc)
{
    if(desc == "signed char" || desc == "int8" || desc == "int8_t")
        return Char;
    if(desc == "uchar" || desc == "unsigned char" || desc == "uint8" || desc == "uint8_t")
        return UChar;
    if(desc == "short" || desc == "short int" || desc == "signed short" ||
       desc == "signed short int" || desc == "int16" || desc == "int16_t")
        return Short;
    if(desc == "ushort" || desc == "unsigned short" || desc == "unsigned short int" ||
       desc == "uint16" || desc == "uint16_t")
        return UShort;
    if(desc == "int" || desc == "signed int" || desc == "int32" || desc == "int32_t")
        return Int;
    if(desc == "uint" || desc == "unsigned int" || desc == "uint32" || desc == "uint32_t")
        return UInt;
    if(desc == "longlong" || desc == "long long" || desc == "long long int" || desc == "int64" ||
       desc == "signed long long" || desc == "signed long long int" || desc == "int64_t")
        return Int64;
    if(desc == "ulonglong" || desc == "unsigned long long" ||
       desc == "unsigned long long int" || desc == "uint64" || desc == "uint64_t")
        return UInt64;
    if(desc == "float")
        return Float;
    if(desc == "double")
        return Double;
    if(desc == "block")
        return Block;

    return DataType(-1); // invalid type
}

template<typename T>
bool NrrdFileIO::checkHeader(const QVariantMap &metaInfo) const
{
    auto fail = [](const QString& s){
        qCritical() << s;
        return false;
    };
    // minimum information that is required
    if(!metaInfo.contains("nrrd header offset") ||
       !metaInfo.contains(meta_info::dimensions) ||
       !metaInfo.contains("data type enum"))
    {
        return fail("insufficient header information");
    }
    // the only supported encoding so far (TBD: add ascii support)
    if(metaInfo.value(_fEncoding).toString().compare("raw", Qt::CaseInsensitive) != 0)
    {
        return fail("unsupported data encoding: " + metaInfo.value(_fEncoding).toString());
    }

    // check data type
    const auto dataTypeFromHeader = DataType(metaInfo.value("data type enum").value<int>());
    if(dataTypeFromHeader < 0 || dataTypeFromHeader > 10)
    {
        return fail("unknown or unsupported data type");
    }
    if(dataType<T>() != dataTypeFromHeader)
    {
        return fail("data type does not fit to nrrd header information");
    }
    if(dataTypeFromHeader == DataType::Block)
    {
        auto blockSize = metaInfo.contains("blocksize")
                ? metaInfo.value("blocksize").toInt()
                : metaInfo.value("block size").toInt();
        if(blockSize <= 0)
            return fail("invalid or missing block size");
        if(blockSize != sizeof(T))
            return fail("block size in nrrd header does not match to requested data type size");
    }
    else if(sizeOfType(dataTypeFromHeader) != sizeof(T))
    {
        return fail("data type size does not fit to nrrd header information");
    }

    // raw data endianness
    if(metaInfo.contains(_fEndianness) && dataTypeFromHeader > int(DataType::UChar))
    {
        auto endianness = metaInfo.value(_fEndianness).toString();
        if(endianness.compare("little", Qt::CaseInsensitive) == 0)
        {
            if(isBigEndian())
                return fail("conversion little to big endian not implemented");
        }
        else if(endianness.compare("big", Qt::CaseInsensitive) == 0)
        {
            if(!isBigEndian())
                return fail("conversion big to little endian not implemented");
        }
        else
        {
            return fail("unknown endianness: " + endianness);
        }
    }

    return true;
}

inline int NrrdFileIO::sizeOfType(DataType type)
{
    // enum DataType { Char, UChar, Short, UShort, Int, UInt, Int64, UInt64, Float, Double, Block };
    const int sizes[] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 0 }; // block size is unspecified (0)
    return sizes[type];
}

inline const char* NrrdFileIO::stringOfType(DataType type)
{
    static const char* const stringList[] = { "int8",
                                              "uint8",
                                              "int16",
                                              "uint16",
                                              "int32",
                                              "uint32",
                                              "int64",
                                              "uint64",
                                              "float",
                                              "double",
                                              "block" };
    return stringList[type];
}

// translate supported data types to enum
template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<char>() { return DataType::Char; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<uchar>() { return DataType::UChar; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<short>() { return DataType::Short; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<ushort>() { return DataType::UShort; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<int>() { return DataType::Int; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<uint>() { return DataType::UInt; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<int64_t>() { return DataType::Int64; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<uint64_t>() { return DataType::UInt64; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<float>() { return DataType::Float; }

template <>
inline NrrdFileIO::DataType NrrdFileIO::dataType<double>() { return DataType::Double; }

template <typename T>
NrrdFileIO::DataType NrrdFileIO::dataType() { return DataType::Block; }

inline bool NrrdFileIO::isBigEndian()
{
    union {
        uint32_t i;
        char c[4];
    } bint = { 0x01020304 };

    return bint.c[0] == 1;
}

} // namespace io
} // namespace CTL
