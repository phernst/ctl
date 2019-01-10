#include "den_utils.h"

namespace CTL {
namespace io {

// determine the type of the binary data from header information and file size
template<class S>
den::Type den::getDataType(S&& fileName) {
    size_t bytesData;
    auto head  = den::loadHeader(std::forward<S>(fileName), &bytesData);
    auto numEl = head.numEl();
    if(     numEl == 0    // header is incomplete or (partly) zero
     || bytesData == 0    // no data or file corruption
     || bytesData%numEl ) // invalid ration between header and data size
        return unDef;

    switch(bytesData/numEl) {
        case uChar  : return uChar;
        case uShort : return uShort;
        case Float  : return Float;
        case Double : return Double;
        default     : return unDef;
    }
}

// save functions with container type deduction
template<class C, class S>
bool den::save(const C &toWrite, S&& fileName, int rows, int cols, int count) {
    return _tpl_deduct::save(toWrite,std::forward<S>(fileName),rows,cols,count);
}
template<class C, class S>
bool den::save(const C &toWrite, S&& fileName, const den::Header &header) {
    return _tpl_deduct::save(toWrite,std::forward<S>(fileName),header.rows,header.cols,header.count);
}

// save function using append mode
template<class C, class S>
bool den::saveAppendMode(const C &toWrite, S&& fileName, int rows, int cols, int count) {
    auto tmpHeader = createHeader(toWrite.size(),rows,cols,count);
    if(tmpHeader.isZero() || tmpHeader.isNegative() || tmpHeader.isOutOfBounds())
        return false;
    return saveAppendMode(toWrite,std::forward<S>(fileName),tmpHeader);
}
template<class C, class S>
bool den::saveAppendMode(const C &toWrite, S&& fileName, const den::Header &header) {
    auto fileHeader = loadHeader(fileName);
    if(fileHeader.isZero())
        return save(toWrite,std::forward<S>(fileName),header);
    if(fileHeader.rows==header.rows && fileHeader.cols==header.cols)
        return appendMatrices(std::forward<S>(fileName),toWrite);
    return false;
}

// append functions with container type deduction
template<template<typename,class...> class C, typename T, class... A, class S>
bool den::appendMatrices(S&& fileName, const C<T,A...> &toAppend, bool padding, T value) {
    return _tpl_deduct::appendMatrices(std::forward<S>(fileName), toAppend, padding, value);
}

// load functions
template<template<typename> class Container, class S>
Container<uchar> den::loadUChar(S&& fileName, uint startMatrix, int numMatrices) {
    DFileFormat<Container> tmp(std::forward<S>(fileName));
    return tmp.loadUChar(startMatrix, numMatrices);
}
template<template<typename> class Container, class S>
Container<ushort> den::loadUShort(S&& fileName, uint startMatrix, int numMatrices) {
    DFileFormat<Container> tmp(std::forward<S>(fileName));
    return tmp.loadUShort(startMatrix, numMatrices);
}
template<template<typename> class Container, class S>
Container<float> den::loadFloat(S&& fileName, uint startMatrix, int numMatrices) {
    DFileFormat<Container> tmp(std::forward<S>(fileName));
    return tmp.loadFloat(startMatrix, numMatrices);
}
template<template<typename> class Container, class S>
Container<double> den::loadDouble(S&& fileName, uint startMatrix, int numMatrices) {
    DFileFormat<Container> tmp(std::forward<S>(fileName));
    return tmp.loadDouble(startMatrix, numMatrices);
}

// convenient function for removing a file
template<class S>
bool den::remove(S&& fileName)
{
    return !std::remove(den::_tpl_deduct::toStdString(std::forward<S>(fileName)));
}

} // namespace io
} // namespace CTL
