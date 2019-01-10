/******************************************************************************
** DFileFormat template class implementation for "dfileformat.h"
**
** by Robert Frysch | Oct 08, 2018
** Otto von Guericke University Magdeburg
** Institute for Medical Engineering - IMT (Head: Georg Rose)
** Email: robert.frysch@ovgu.de
******************************************************************************/

#include "dfileformat.h"
#include <limits>
#include <iostream>

// checks format of floating point numbers at compile time and leads to a compiler
// error if the platform does not support floating point numbers according to IEEE 754
static_assert(std::numeric_limits<float>::is_iec559, "'float' must be compliant with the IEEE 754 standard");
static_assert(std::numeric_limits<double>::is_iec559, "'double' must be compliant with the IEEE 754 standard");
static_assert(sizeof(float)  * CHAR_BIT == 32, "'float' must be 32 Bit");
static_assert(sizeof(double) * CHAR_BIT == 64, "'double' must be 64 Bit");

namespace CTL {
namespace io {
namespace den {

// constructors
template<template<typename> class Container>
DFileFormat<Container>::DFileFormat()
{ }
template<template<typename> class Container>
template<class S>
DFileFormat<Container>::DFileFormat(S&& fileName)
    : fn(den::_tpl_deduct::toStdString(std::forward<S>(fileName)))
{ }

// setter
template<template<typename> class Container>
template<class S>
void DFileFormat<Container>::setFileName(S&& toSet) {
    fn = den::_tpl_deduct::toStdString(std::forward<S>(toSet));
}
template<template<typename> class Container>
void DFileFormat<Container>::setVerbose(bool toSet) {
    v = toSet;
}

// save
template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<uchar>  &toWrite, int rows, int cols, int count) const {
    return gen_save(toWrite, rows, cols, count);
}
template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<ushort> &toWrite, int rows, int cols, int count) const {
    return gen_save(toWrite, rows, cols, count);
}
template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<float>  &toWrite, int rows, int cols, int count) const {
    return gen_save(toWrite, rows, cols, count);
}
template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<double> &toWrite, int rows, int cols, int count) const {
    return gen_save(toWrite, rows, cols, count);
}

template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<uchar>  &toWrite, const den::Header &header) const {
    return gen_save(toWrite, header.rows, header.cols, header.count);
}
template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<ushort> &toWrite, const den::Header &header) const {
    return gen_save(toWrite, header.rows, header.cols, header.count);
}
template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<float>  &toWrite, const den::Header &header) const {
    return gen_save(toWrite, header.rows, header.cols, header.count);
}
template<template<typename> class Container>
bool DFileFormat<Container>::save(const Container<double> &toWrite, const den::Header &header) const {
    return gen_save(toWrite, header.rows, header.cols, header.count);
}

// append
template<template<typename> class Container>
bool DFileFormat<Container>::appendMatrices(const Container<uchar>  &toAppend, bool padding, uchar  value) {
    return gen_append(toAppend, padding, value);
}
template<template<typename> class Container>
bool DFileFormat<Container>::appendMatrices(const Container<ushort> &toAppend, bool padding, ushort value) {
    return gen_append(toAppend, padding, value);
}
template<template<typename> class Container>
bool DFileFormat<Container>::appendMatrices(const Container<float>  &toAppend, bool padding, float  value) {
    return gen_append(toAppend, padding, value);
}
template<template<typename> class Container>
bool DFileFormat<Container>::appendMatrices(const Container<double> &toAppend, bool padding, double value) {
    return gen_append(toAppend, padding, value);
}

// load
template<template<typename> class Container>
Container<uchar>  DFileFormat<Container>::loadUChar(uint startMatrix, int numMatrices)  {
    return gen_load<uchar>(startMatrix, numMatrices);
}
template<template<typename> class Container>
Container<ushort> DFileFormat<Container>::loadUShort(uint startMatrix, int numMatrices) {
    return gen_load<ushort>(startMatrix, numMatrices);
}
template<template<typename> class Container>
Container<float>  DFileFormat<Container>::loadFloat(uint startMatrix, int numMatrices)  {
    return gen_load<float>(startMatrix, numMatrices);
}
template<template<typename> class Container>
Container<double> DFileFormat<Container>::loadDouble(uint startMatrix, int numMatrices) {
    return gen_load<double>(startMatrix, numMatrices);
}

template<template<typename> class Container>
const den::Header& DFileFormat<Container>::loadHeader() {
    return lastReadIn = den::loadHeader(fn);
}

template<template<typename> class Container>
void DFileFormat<Container>::callBack(const char *output, int code, int numSteps) const
{
    using std::cout;
    using std::cerr;
    using std::endl;

    if(numSteps && v) // progress case:
    {
        ++numSteps;
        if(!code) cout << output  << " ";
        cout << code*100/numSteps << "%";
        if(code==numSteps)  cout  << endl;
        else                cout  << " ";
    }
    else // standard output, warnings, errors (numSteps==0):
    {
        if      (code>0 && v) cout << "Warning: " << output << endl;
        else if (code<0)      cerr << "Error: "   << output << endl
                                   << fn.data()   << endl;
        else if (v) cout << output << endl; // standard output (code==0)
    }
}

template<template<typename> class Container>
template <typename T>
bool DFileFormat<Container>::gen_save(const Container<T> &toWrite, int rows, int cols, int count) const
{
    constexpr int64_t GB = 1024*1024*1024; // data chunk for block write
    callBack("<< store data to DEN file using the path");
    callBack(fn.data());
    if(toWrite.size() == 0) return callBack("vector is empty",-1),false;
    auto header = den::createHeader(toWrite.size(), rows, cols, count);
    callBack(header.info().data());
    if(header.isNegative())
        return callBack("at least one header dimension is negative",-1),false;
    if(header.isZero())
        return callBack("passed sizes do not fit to vector size",-1),false;
    if(header.isOutOfBounds())
        return callBack("at least one header dimension exceeds 16 bit (65'535)",-1),false;

    std::ofstream file(fn,std::ios::binary);
    if(!file.is_open()) return callBack("cannot open file",-1),false;

    auto bytes2write      = static_cast<int64_t>(header.numEl()*sizeof(T));
    const auto numBlocks  = static_cast<int>(bytes2write/GB);

    const ushort fHead[3] = {static_cast<ushort>(header.rows),
                             static_cast<ushort>(header.cols),
                             static_cast<ushort>(header.count)};
    file.write(reinterpret_cast<const char*>(fHead),6);
    if(file.fail()) return callBack("writing the header fails",-1),false;

    auto ctr = 0;
    auto pos = reinterpret_cast<const char*>(toWrite.data());
    callBack("write raw data...");
    do{ file.write(pos,minimum(bytes2write,GB));
        if(file.fail()) return callBack("writing the data fails",-1),false;
        bytes2write -= GB;
        pos += GB;
        if(numBlocks) callBack("write raw data...",++ctr,numBlocks);
    }while(bytes2write>0);
    file.close();
    callBack("data saved successfully.");
    return true;
}

template<template<typename> class Container>
template <typename T>
bool DFileFormat<Container>::gen_append(const Container<T> &toAppend, bool padding, T value)
{
    constexpr int64_t GB = 1024*1024*1024; // data chunk for block write
    size_t bytesData;
    callBack("<< append data to DEN file with the path");
    callBack(fn.data());
    lastReadIn = den::loadHeader(fn, &bytesData);
    callBack(lastReadIn.info().data());
    auto matSize = static_cast<unsigned int>(lastReadIn.rows)*static_cast<unsigned int>(lastReadIn.cols);
    auto numEl = lastReadIn.numEl();
    auto newEl = toAppend.size();
    if(numEl == 0 || bytesData == 0 || bytesData%numEl) return callBack("file corruption",-1),false;
    if(bytesData/numEl != sizeof(T))                    return callBack("data type does not fit",-1),false;
    if(newEl == 0)                                      return true;  // nothing to append
    unsigned int rest = newEl % matSize;
    if(rest && !padding)                                return callBack("number of elements does not fit to matrix dimension (no-padding case)",-1),false;

    // open file in append mode & write 'toAppend'
    std::ofstream file(fn,std::ios::binary | std::ios::ate | std::ios::in);
    if(!file.is_open()) return callBack("cannot open file",-1),false;
    auto bytes2write     = static_cast<int64_t>(newEl*sizeof(T));
    const auto numBlocks = static_cast<int>(bytes2write/GB);
    auto ctr = 0;
    auto pos = reinterpret_cast<const char*>(toAppend.data());
    callBack("append raw data...");
    do{ file.write(pos,minimum(bytes2write,GB));
        if(file.fail()) return callBack("writing the data fails",-1),false;
        bytes2write -= GB;
        pos += GB;
        if(numBlocks) callBack("append raw data...",++ctr,numBlocks);
    }while(bytes2write>0);
    lastReadIn.count += static_cast<int>(newEl / matSize);

    // padding
    if(rest) {
        auto numPads = matSize - rest;
        // write 'T value' numPads times
        for(unsigned int i = 0; i < numPads; ++i) {
            file.write(reinterpret_cast<const char*>(&value),sizeof(T));
            if(file.fail()) return callBack("padding fails",-1),false;
        }
        ++lastReadIn.count;
    }

    // update header
    const ushort fHead[3] = {static_cast<ushort>(lastReadIn.rows),
                             static_cast<ushort>(lastReadIn.cols),
                             static_cast<ushort>(lastReadIn.count)};
    file.seekp(0);
    file.write(reinterpret_cast<const char*>(fHead),6);
    if(file.fail()) return callBack("writing the header fails",-1),false;

    file.close();
    callBack("new file dimensions:");
    callBack(lastReadIn.info().data());
    return true;
}

template<template<typename> class Container>
template <typename T>
Container<T> DFileFormat<Container>::gen_load(uint startMatrix, int numMatrices)
{
    constexpr int64_t GB = 1024*1024*1024; // data chunk for block read

    // read header and check file consistency
    callBack(">> load from DEN file using the path");
    callBack(fn.data());
    std::ifstream file(fn,std::ios::binary | std::ios::ate);
    if(!file.is_open()) return callBack("cannot open file",-1),Container<T>();
    const int64_t bytesOfFile = file.tellg();
    if(bytesOfFile < 6) return callBack("no complete header available",-1),Container<T>();

    ushort fHead[3];
    file.seekg(0);
    file.read(reinterpret_cast<char*>(fHead),6);
    if(file.fail()) return callBack("reading the header fails",-1),Container<T>();
    lastReadIn = den::Header(fHead[0],fHead[1],fHead[2]);
    if(lastReadIn.isZero()) return callBack("invalid header",-1),Container<T>();

    auto nbElements = lastReadIn.numEl();
    auto bytes2read = static_cast<int64_t>(nbElements*sizeof(T));
    if(bytes2read != (bytesOfFile-6))
        return callBack("header does not fit to file size",-1),Container<T>();

    // evaluate arguments
    if(numMatrices == 0) return Container<T>();
    if(startMatrix >= static_cast<uint>(lastReadIn.count))
        return callBack("startMatrix exceeds number of matrices in file",-1),Container<T>();
    auto numMatrixElements = static_cast<size_t>(lastReadIn.rows)*static_cast<size_t>(lastReadIn.cols);
    auto bytesSingleMatrix = numMatrixElements*sizeof(T);
    auto offSetBytes = startMatrix*bytesSingleMatrix;
    bytes2read -= offSetBytes;
    nbElements -= startMatrix*numMatrixElements;
    if(numMatrices > 0) {
        auto requestedBytes = numMatrices*static_cast<int64_t>(bytesSingleMatrix);
        if(requestedBytes > bytes2read)
            return callBack("more matrices requested than available",-1),Container<T>();
        bytes2read = requestedBytes;
        nbElements = numMatrices*numMatrixElements;
    }
    else if(numMatrices != -1) {
        callBack("illegal function argument when loading the file",-1);
        callBack(("numMatrices = " + std::to_string(numMatrices)).data());
        return Container<T>();
    }

    // allocate container and read the data to it
    Container<T> ret(nbElements);
    const auto numBlocks = static_cast<int>(bytes2read/GB);
    auto ctr = 0;
    auto pos = reinterpret_cast<char*>(ret.data());
    callBack("read raw data...");
    file.seekg(offSetBytes, std::ios::cur);
    do{ file.read(pos,minimum(bytes2read,GB));
        if(file.fail()) return callBack("reading the data fails",-1),Container<T>();
        bytes2read -= GB;
        pos += GB;
        if(numBlocks) callBack("read raw data...",++ctr,numBlocks);
    }while(bytes2read>0);
    file.close();
    if(startMatrix!=0 || numMatrices>0) {
        auto output = std::to_string(lastReadIn.rows) + " x " +
                      std::to_string(lastReadIn.cols) + " x " +
                      std::to_string(nbElements/(lastReadIn.rows*lastReadIn.cols)) +
                      " elements";
        callBack(output.data());
    }
    callBack("successfully loaded from a file with size:");
    callBack(lastReadIn.info().data());
    return ret;
}

} // namespace den
} // namespace io
} // namespace CTL
