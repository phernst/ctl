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
    // Nrrd fields which are translated to basetype meta info
    static const QString dimension = QStringLiteral("dimension");
    static const QString sizes = QStringLiteral("sizes");
    static const QString type = QStringLiteral("type");
    static const QString encoding = QStringLiteral("encoding");
    static const QString thicknesses = QStringLiteral("thicknesses");
    static const QString spaceOrigin = QStringLiteral("space origin");
    static const QString labels = QStringLiteral("labels");

    if(field.compare(dimension, Qt::CaseInsensitive) == 0)
    {
        if(*nbDimension)
            return false;
        *nbDimension = desc.toInt();
        return 0 < *nbDimension && *nbDimension <= 4;
    }
    else if(field.compare(sizes, Qt::CaseInsensitive) == 0)
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
    else if(field.compare(type, Qt::CaseInsensitive) == 0)
    {
        if(metaInfo->contains(type))
            return false;
        auto dataTypeID = dataTypeFromString(desc);
        if(dataTypeID < 0)
            return false;
        metaInfo->insert(type, desc);
        metaInfo->insert("data type enum", dataTypeID);
    }
    else if(field.compare(encoding, Qt::CaseInsensitive) == 0)
    {
        if(metaInfo->contains(encoding))
            return false;

        if(desc == "raw")
            metaInfo->insert(encoding, "raw");
        else if(desc == "txt" || desc == "text" || desc == "ascii")
            metaInfo->insert(encoding, "ascii");
    }
    else if(field.compare(thicknesses, Qt::CaseInsensitive) == 0)
    {
        if(metaInfo->contains(meta_info::voxSizeX))
            return false;

        auto thicknesses = desc.split(' ');
        metaInfo->insert(meta_info::voxSizeX, thicknesses.value(0).toFloat());
        metaInfo->insert(meta_info::voxSizeY, thicknesses.value(1).toFloat());
        metaInfo->insert(meta_info::voxSizeZ, thicknesses.value(2).toFloat());
    }
    else if(field.compare(spaceOrigin, Qt::CaseInsensitive) == 0)
    {
    //space origin: (0.0,1.0,0.3)
        if(metaInfo->contains(meta_info::volOffX))
            return false;
        if(!desc.startsWith('(') || !desc.endsWith(')'))
            return false;
        auto offSet = desc.mid(1);
        offSet.chop(1);
        auto vector = desc.split(',');
        metaInfo->insert(meta_info::volOffX, vector.value(0).toFloat());
        metaInfo->insert(meta_info::volOffY, vector.value(1).toFloat());
        metaInfo->insert(meta_info::volOffZ, vector.value(2).toFloat());
    }
    else if(field.compare(labels, Qt::CaseInsensitive) == 0)
    {
    //labels: "<label[0]>" "<label[1]>" ... "<label[dim-1]>"
        if(metaInfo->contains(meta_info::dim1Type))
            return false;

        auto dimTypes = desc.split(' ');
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

    std::ifstream is(fileName.toStdString(), std::ios::binary | std::ios::ate);
    if(!is)
    {
        qCritical() << "unable to open file" << fileName;
        return ret;
    }

    // get length data block
    const int64_t bytesOfFile = is.tellg();
    const int64_t dataBytes = bytesOfFile - headerOffset;
    const int64_t nbElements = dimensions.dim1 * dimensions.dim2 *
            (dimensions.nbDim >= 3 ? dimensions.dim3 : 1) *
            (dimensions.nbDim == 4 ? dimensions.dim4 : 1);

    if(nbElements * sizeof(T) != dataBytes)
    {
        qCritical() << "raw data size of file does not fit to dimensions in nrrd header";
        return ret;
    }

    ret.resize(nbElements);
    is.seekg(headerOffset);
    is.read(reinterpret_cast<char*>(ret.data()), dataBytes);

    if(!is)
        qCritical() << "only" << is.gcount() << "could be read";

    is.close();
    return ret;
}

template<typename T>
bool NrrdFileIO::checkHeader(const QVariantMap &metaInfo)
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
    if(metaInfo.value("encoding").toString().compare("raw", Qt::CaseInsensitive))
    {
        return fail("unsupported data encoding:" + metaInfo.value("encoding").toString());
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
    if(metaInfo.contains("endian") && dataTypeFromHeader > int(DataType::UChar))
    {
        auto endianness = metaInfo.value("endian").toString();
        if(endianness.compare("little", Qt::CaseInsensitive))
        {
            if(isBigEndian())
                return fail("conversion little to big endian not implemented");
        }
        else if(endianness.compare("big", Qt::CaseInsensitive))
        {
            if(!isBigEndian())
                return fail("conversion big to little endian not implemented");
        }
        else
        {
            return fail("unknown endianness:" + endianness);
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
