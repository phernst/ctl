/******************************************************************************
** Provides den::Header struct to store and evaluate size information
** of 3d data, specially dedicated to the header of a DEN file.
**
** by Robert Frysch | Jul 11, 2018
** Otto von Guericke University Magdeburg
** Institute for Medical Engineering - IMT (Head: Georg Rose)
** Email: robert.frysch@ovgu.de
******************************************************************************/

#ifndef HEADER_H
#define HEADER_H

#include <climits>
#include <string>

namespace CTL {
namespace io {
namespace den {

// Header class for storing "rows / columns / count" and some auxiliary functions
struct Header;

// __Factory functions__
// creates Header from size information (potentially partially specified)
Header createHeader(size_t totSize, int rows = 0, int cols = 0, int count = 0);
// load only header information from file
template <class S>
Header loadHeader(S&& fileName, size_t* bytesData = nullptr);

// __Header declaration__
struct Header
{
    // constructors
    Header();
    Header(int rows, int cols, int count);

    // factories
    static Header fromTotalSize(size_t totSize, int rows = 0, int cols = 0, int count = 0);
    template <class S>
    static Header fromFile(S&& fileName, size_t* bytesData = nullptr);

    // const methods
    size_t numEl() const;

    bool operator!=(size_t cmp) const;
    bool operator==(size_t cmp) const;

    bool isZero() const;
    bool isOutOfBounds() const;
    bool isNegative() const;

    std::string info() const;

    // public members
    int rows, cols, count;
};

// __Inline definitions__
inline Header::Header()
    : rows(0)
    , cols(0)
    , count(0)
{
}

inline Header::Header(int rows, int cols, int count)
    : rows(rows)
    , cols(cols)
    , count(count)
{
}

inline Header Header::fromTotalSize(size_t totSize, int rows, int cols, int count)
{
    return createHeader(totSize, rows, cols, count);
}

inline size_t Header::numEl() const
{
    return static_cast<size_t>(rows) * static_cast<size_t>(cols) * static_cast<size_t>(count);
}

inline bool Header::operator!=(size_t cmp) const { return cmp != numEl(); }

inline bool Header::operator==(size_t cmp) const { return cmp == numEl(); }

inline bool Header::isZero() const { return *this == 0; }

inline bool Header::isOutOfBounds() const
{
    return rows>USHRT_MAX  ? true
         : cols>USHRT_MAX  ? true
         : count>USHRT_MAX ? true
         : false;
}

inline bool Header::isNegative() const
{
    return rows<0  ? true
         : cols<0  ? true
         : count<0 ? true
         : false;
}

inline std::string Header::info() const
{
    return std::to_string(rows) + " x " + std::to_string(cols) + " x " + std::to_string(count);
}

} // namespace den
} // namespace io
} // namespace CTL

#include "den_header.tpp"

#endif // HEADER_H
