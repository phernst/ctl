/******************************************************************************
** Template implementation for "matrix.h"
** by Robert Frysch | Feb 05, 2018
** Otto von Guericke University Magdeburg
** Institute for Medical Engineering - IMT (Head: Georg Rose)
** Email: robert.frysch@ovgu.de
******************************************************************************/

#include "matrix.h" // optional, only for the IDE
#include <stdexcept>

namespace CTL {
namespace mat {

// ### MatrixBase ###
// constructors
template <uint Rows, uint Cols>
MatrixBase<Rows, Cols>::MatrixBase(double fillValue)
{
    std::fill(this->begin(), this->end(), fillValue);
}

template <uint Rows, uint Cols>
inline MatrixBase<Rows, Cols>::MatrixBase(const double (&initArray)[Rows * Cols])
{
    std::copy_n(initArray, Rows * Cols, _m);
}

// individual element access with 2 indizes
// -> run time boundary check (throws out_of_range)
template <uint Rows, uint Cols>
double& MatrixBase<Rows, Cols>::at(uint row, uint column) noexcept(false)
{
    if(row >= Rows)
        throw std::out_of_range("row index exceeds matrix dimensions");
    if(column >= Cols)
        throw std::out_of_range("column index exceeds matrix dimensions");
    return (*this)[row][column];
}
template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::at(uint row, uint column) const noexcept(false)
{
    if(row >= Rows)
        throw std::out_of_range("row index exceeds matrix dimensions");
    if(column >= Cols)
        throw std::out_of_range("column index exceeds matrix dimensions");
    return (*this)[row][column];
}
// -> compile time boundary check (never fails)
template <uint Rows, uint Cols>
template <uint row, uint column>
double& MatrixBase<Rows, Cols>::get() noexcept
{
    static_assert(row < Rows, "row index must not exceed matrix dimension");
    static_assert(column < Cols, "column index must not exceed matrix dimension");
    return (*this)[row][column];
}
template <uint Rows, uint Cols>
template <uint row, uint column>
double MatrixBase<Rows, Cols>::get() const noexcept
{
    static_assert(row < Rows, "row index must not exceed matrix dimension");
    static_assert(column < Cols, "column index must not exceed matrix dimension");
    return (*this)[row][column];
}

// individual element access with 1 index
// -> run time boundary check (throws out_of_range)
template <uint Rows, uint Cols>
double& MatrixBase<Rows, Cols>::at(uint n) noexcept(false)
{
    if(n >= Rows * Cols)
        throw std::out_of_range("index exceeds matrix dimensions");
    return _m[n];
}
template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::at(uint n) const noexcept(false)
{
    if(n >= Rows * Cols)
        throw std::out_of_range("index exceeds matrix dimensions");
    return _m[n];
}
// -> compile time boundary check (never fails)
template <uint Rows, uint Cols>
template <uint n>
double& MatrixBase<Rows, Cols>::get() noexcept
{
    static_assert(n < Rows * Cols, "index must not exceed matrix dimensions");
    return _m[n];
}
template <uint Rows, uint Cols>
template <uint n>
double MatrixBase<Rows, Cols>::get() const noexcept
{
    static_assert(n < Rows * Cols, "index must not exceed matrix dimensions");
    return _m[n];
}

// formatting matrix entries to a string
template <uint Rows, uint Cols>
std::string MatrixBase<Rows, Cols>::info(const char* lineModifier) const
{
    // number of spaces (SEPARATOR_CHARACTER_FOR_INFO_STRING) between numbers
    // -> must be at least 3, otherwise last column will be truncated
    static const uint numSpacesInBetween = 3;
    // prpare begin of each line (row) using lineModifier
    size_t lineModSize = std::char_traits<char>::length(lineModifier);
    std::string lineBegin(lineModSize + 1, '|');
    lineBegin.replace(0, lineModSize, lineModifier);
    // allocate array of strings for the single matrix entries (converted numbers)
    std::string entries[Rows * Cols];
    std::string* entryPtr = entries;
    // find maximum length of the of the entries' string representation
    size_t maxLength = 0;
    for(auto val : _m)
    {
        *entryPtr = std::to_string(val);
        maxLength = std::max(maxLength, entryPtr->size());
        ++entryPtr;
    }
    maxLength += numSpacesInBetween;

    // build result string (copy all single entries to the correct position)
    size_t rowSize = Cols * maxLength + lineModSize;
    std::string ret(Rows * rowSize, SEPARATOR_CHARACTER_FOR_INFO_STRING);
    entryPtr = entries;
    for(uint i = 0; i < Rows; ++i)
    {
        size_t rowOffSet = i * rowSize;
        for(uint j = 0; j < Cols; ++j)
        {
            // column offset: needs constant offset for '|' and `lineModifier`
            size_t colOffSet = j * maxLength + 1 + lineModSize;
            size_t indentation = maxLength - entryPtr->size() - numSpacesInBetween;
            ret.replace(rowOffSet + colOffSet + indentation, entryPtr->size(), *entryPtr);
            ++entryPtr;
        }
        // write begin of the line (row starts with `lineModifier` + '|')
        ret.replace(rowOffSet, lineBegin.size(), lineBegin);
        // write end of the line (row ends with '|' + newline)
        ret.replace(rowOffSet + rowSize - 2, 2, "|\n");
    }
    return ret;
}

template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::norm() const
{
#ifndef ENABLE_FROBENIUS_NORM
    static_assert(Rows == 1 || Cols == 1,
                  "norm is currently only defined for vectors/scalars. "
                  "For using 'norm()' as the Frobenius norm of a matrix, "
                  "please define 'ENABLE_FROBENIUS_NORM' before including 'matrix.h'.");
#endif
    double ret = 0.0;
    for(auto val : _m)
        ret += val * val;
    return sqrt(ret);
}

// Test that all elements are equal, it means have the equal representation of all elements
template <uint Rows, uint Cols>
bool MatrixBase<Rows, Cols>::operator==(const MatrixBase<Rows, Cols>& other) const
{
    auto otherPtr = other.begin();
    for(auto val : _m)
    {
        if(val != *otherPtr)
            return false;
        ++otherPtr;
    }
    return true;
}

// Test that there is at least one element that is not equal for both matrices, it means does not
// have the equal representation of all elements
template <uint Rows, uint Cols>
bool MatrixBase<Rows, Cols>::operator!=(const MatrixBase<Rows, Cols>& other) const
{
    return !(*this == other);
}

// ### Matrix ###
// ctors
template <uint Rows, uint Cols>
Matrix<Rows, Cols>::Matrix(double fillValue)
    : MatrixBase<Rows, Cols>(fillValue)
{
}

template <uint Rows, uint Cols>
inline Matrix<Rows, Cols>::Matrix(const double (&initArray)[Rows * Cols])
    : MatrixBase<Rows, Cols>(initArray)
{
}

template <uint Rows, uint Cols>
template<typename... Doubles, typename>
inline Matrix<Rows, Cols>::Matrix(double firstElement, Doubles... matrixElements)
    : MatrixBase<Rows, Cols>({ firstElement, static_cast<double>(matrixElements)... })
{
}

// factory
template <uint Rows, uint Cols>
template <class Container>
Matrix<Rows, Cols>
Matrix<Rows, Cols>::fromContainer(const Container& vector, size_t NthMat, bool* ok)
{
    auto offSet = NthMat * Rows * Cols;
    if(offSet + Rows * Cols > static_cast<size_t>(vector.size()))
    {
        if(ok)
            *ok = false;
        return Matrix<Rows, Cols>(0.0);
    }

    Matrix<Rows, Cols> ret;
    auto vecIt = vector.begin() + offSet;
    for(auto& val : ret)
    {
        val = static_cast<double>(*vecIt);
        ++vecIt;
    }
    if(ok)
        *ok = true;
    return ret;
}

// sub-matrix extraction
template <uint Rows, uint Cols>
template <uint fromRow, uint toRow, uint fromCol, uint toCol>
auto Matrix<Rows, Cols>::subMat() const
-> Matrix<toRow >= fromRow ? toRow - fromRow + 1 : fromRow - toRow + 1,
          toCol >= fromCol ? toCol - fromCol + 1 : fromCol - toCol + 1>
{
    static_assert(fromRow < Rows, "fromRow exceeds matrix dimension");
    static_assert(toRow < Rows, "toRow exceeds matrix dimension");
    static_assert(fromCol < Cols, "fromCol exceeds matrix dimension");
    static_assert(toCol < Cols, "toCol exceeds matrix dimension");
    constexpr auto nbRowsSub = toRow >= fromRow ? toRow - fromRow + 1 : fromRow - toRow + 1;
    constexpr auto nbColsSub = toCol >= fromCol ? toCol - fromCol + 1 : fromCol - toCol + 1;

    Matrix<nbRowsSub, nbColsSub> ret;
    constexpr int rowInc = toRow >= fromRow ? 1 : -1;
    constexpr int colInc = toCol >= fromCol ? 1 : -1;
    uint row = fromRow, subRow = 0;
    uint col = fromCol, subCol = 0;

    for(row = fromRow, subRow = 0; row != (toRow + rowInc); row += rowInc, ++subRow)
        for(col = fromCol, subCol = 0; col != (toCol + colInc); col += colInc, ++subCol)
            ret(subRow, subCol) = (*this)(row, col);

    return ret;
}

// single vector extraction
template <uint Rows, uint Cols>
template <uint i>
Matrix<1, Cols> Matrix<Rows, Cols>::row() const
{
    static_assert(i < Rows, "row index must not exceed matrix dimensions");
    Matrix<1, Cols> ret;
    std::copy_n((*this)[i], Cols, ret.begin());
    return ret;
}

template <uint Rows, uint Cols>
template <uint j>
Matrix<Rows, 1> Matrix<Rows, Cols>::column() const
{
    static_assert(j < Cols, "column index must not exceed matrix dimensions");
    Matrix<Rows, 1> ret;
    auto scrPtr = this->constBegin() + j;
    for(auto& val : ret)
    {
        val = *scrPtr;
        scrPtr += Cols;
    }
    return ret;
}

// unary operators
template <uint Rows, uint Cols>
Matrix<Cols, Rows> Matrix<Rows, Cols>::transposed() const
{
    Matrix<Cols, Rows> ret;
    auto scrPtr = this->constBegin();
    uint row, column;

    for(row = 0; row < Rows; ++row)
        for(column = 0; column < Cols; ++column)
        {
            ret(column, row) = *scrPtr;
            ++scrPtr;
        }
    return ret;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator-() const
{
    Matrix<Rows, Cols> ret;
    auto scrPtr = this->constBegin();
    for(auto& val : ret)
    {
        val = -*scrPtr;
        ++scrPtr;
    }
    return ret;
}

// compound assignment
template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator*=(double scalar)
{
    for(auto& val : *this)
        val *= scalar;
    return *this;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator/=(double scalar)
{
    for(auto& val : *this)
        val /= scalar;
    return *this;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator+=(const Matrix<Rows, Cols>& other)
{
    auto otherPtr = other.begin();
    for(auto& val : *this)
    {
        val += *otherPtr;
        ++otherPtr;
    }
    return *this;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator-=(const Matrix<Rows, Cols>& other)
{
    auto otherPtr = other.begin();
    for(auto& val : *this)
    {
        val -= *otherPtr;
        ++otherPtr;
    }
    return *this;
}

// binary operators
template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator*(double scalar) const
{
    Matrix<Rows, Cols> ret;
    auto lhsPtr = this->constBegin();
    for(auto& val : ret)
    {
        val = *lhsPtr * scalar;
        ++lhsPtr;
    }
    return ret;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator/(double scalar) const
{
    Matrix<Rows, Cols> ret;
    auto lhsPtr = this->constBegin();
    for(auto& val : ret)
    {
        val = *lhsPtr / scalar;
        ++lhsPtr;
    }
    return ret;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator+(const Matrix<Rows, Cols>& rhs) const
{
    Matrix<Rows, Cols> ret;
    auto lhsPtr = this->constBegin();
    auto rhsPtr = rhs.constBegin();
    for(auto& val : ret)
    {
        val = *lhsPtr + *rhsPtr;
        ++lhsPtr;
        ++rhsPtr;
    }
    return ret;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator-(const Matrix<Rows, Cols>& rhs) const
{
    Matrix<Rows, Cols> ret;
    auto lhsPtr = this->constBegin();
    auto rhsPtr = rhs.constBegin();
    for(auto& val : ret)
    {
        val = *lhsPtr - *rhsPtr;
        ++lhsPtr;
        ++rhsPtr;
    }
    return ret;
}

// standard matrix multiplication
template <uint Rows1, uint Cols1_Rows2>
template <uint Cols2>
Matrix<Rows1, Cols2> Matrix<Rows1, Cols1_Rows2>::
operator*(const Matrix<Cols1_Rows2, Cols2>& rhs) const
{
    const Matrix<Rows1, Cols1_Rows2>& lhs = *this;
    Matrix<Rows1, Cols2> ret;
    auto retPtr = ret.begin();
    const double* lhsPtr;
    const double* rhsPtr;
    double val;
    uint row, column, i;

    for(row = 0; row < Rows1; ++row)
        for(column = 0; column < Cols2; ++column)
        {
            val = 0.0;
            lhsPtr = lhs[row];
            rhsPtr = rhs[0] + column;
            for(i = 0; i < Cols1_Rows2; ++i)
            {
                val += *lhsPtr * *rhsPtr;
                lhsPtr += 1;
                rhsPtr += Cols2;
            }
            *retPtr = val;
            ++retPtr;
        }
    return ret;
}

} // namespace mat
} // namespace CTL
