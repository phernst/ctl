#ifndef DENUTILS_H
#define DENUTILS_H

#include "dfileformat.h"

namespace CTL {
namespace io {
namespace den {

// __Types__

// possible types of the binary data
enum Type { unDef = 0, uChar = 1, uShort = 2, Float = 4, Double = 8 };

// __Functions__

// determine the type of the binary data from header information and file size
template <class S>
Type getDataType(S&& fileName);

// save functions with container type deduction
template <class C, class S>
bool save(const C& toWrite, S&& fileName, int rows = 0, int cols = 0, int count = 0);
template <class C, class S>
bool save(const C& toWrite, S&& fileName, const den::Header& header);

// save function using append mode
template <class C, class S>
bool saveAppendMode(const C& toWrite, S&& fileName, int rows = 0, int cols = 0, int count = 0);
template <class C, class S>
bool saveAppendMode(const C& toWrite, S&& fileName, const den::Header& header);

// append functions with container type deduction
template <template <typename, class...> class C, typename T, class... A, class S>
bool appendMatrices(S&& fileName, const C<T, A...>& toAppend, bool padding = false, T value = 0);

// load functions (Container needs to be specified)
template <template <typename> class Container, class S>
Container<uchar> loadUChar(S&& fileName, uint startMatrix = 0, int numMatrices = -1);
template <template <typename> class Container, class S>
Container<ushort> loadUShort(S&& fileName, uint startMatrix = 0, int numMatrices = -1);
template <template <typename> class Container, class S>
Container<float> loadFloat(S&& fileName, uint startMatrix = 0, int numMatrices = -1);
template <template <typename> class Container, class S>
Container<double> loadDouble(S&& fileName, uint startMatrix = 0, int numMatrices = -1);

// convenient function for removing a file
template <class S>
bool remove(S&& fileName);

} // namespace den
} // namespace io
} // namespace CTL

#include "den_utils.tpp"

#endif // DENUTILS_H
