/******************************************************************************
** Template implementation for "matrix.h"
**
** by Robert Frysch
** Otto von Guericke University Magdeburg
** Institute for Medical Engineering - IMT (Head: Georg Rose)
** Email: robert.frysch@ovgu.de
******************************************************************************/

#include "matrix.h" // optional, only for the IDE

namespace CTL {
namespace mat {

// ### MatrixBase ###
// constructors
/*!
 * Construct an instance and initialize all elements with a \a fillValue.
 */
template <uint Rows, uint Cols>
inline MatrixBase<Rows, Cols>::MatrixBase(double fillValue)
{
    std::fill(this->begin(), this->end(), fillValue);
}

/*!
 * Construct an instance from a C-style array. The array length must match the total number of
 * matrix elements, otherwise it results in a compilation error.
 */
template <uint Rows, uint Cols>
inline MatrixBase<Rows, Cols>::MatrixBase(const double (&initArray)[Rows * Cols])
{
    std::copy_n(initArray, Rows * Cols, _m);
}

/*!
 * Directly initialize the underlying C-style array `double[]` using the passed elements.
 */
template <uint Rows, uint Cols>
template <typename... Doubles>
inline MatrixBase<Rows, Cols>::MatrixBase(double firstElement, Doubles... matrixElements)
    : _m{ firstElement, matrixElements... }
{
}

// accessors
/*!
 * Returns a pointer to the first element in a \a row. A run time boundary check is not performed.
 * Elements are stored in row major order.
 */
template <uint Rows, uint Cols>
double* MatrixBase<Rows, Cols>::operator[](uint row)
{
    return _m + row * Cols;
}
/*!
 * Returns a pointer to the first element in a \a row. A run time boundary check is not performed.
 * Elements are stored in row major order.
 */
template <uint Rows, uint Cols>
const double* MatrixBase<Rows, Cols>::operator[](uint row) const
{
    return _m + row * Cols;
}

/*!
 * Returns a reference to the element with index (\a row, \a column). A run time boundary check
 * is not performed.
 */
template <uint Rows, uint Cols>
double& MatrixBase<Rows, Cols>::operator()(uint row, uint column)
{
    return (*this)[row][column];
}
/*!
 * Returns the element with index (\a row, \a column). A run time boundary check
 * is not performed.
 */
template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::operator()(uint row, uint column) const
{
    return (*this)[row][column];
}

/*!
 * Returns a reference to the element with index (\a row, \a column). A run time boundary check
 * is performed and an exception is thrown if an index exceeds the matrix dimensions (throws
 * out_of_range).
 */
template <uint Rows, uint Cols>
double& MatrixBase<Rows, Cols>::at(uint row, uint column) noexcept(false)
{
    if(row >= Rows)
        throw std::out_of_range("row index exceeds matrix dimensions");
    if(column >= Cols)
        throw std::out_of_range("column index exceeds matrix dimensions");
    return (*this)[row][column];
}
/*!
 * Returns the element with index (\a row, \a column). A run time boundary check
 * is performed and an exception is thrown if an index exceeds the matrix dimensions (throws
 * out_of_range).
 */
template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::at(uint row, uint column) const noexcept(false)
{
    if(row >= Rows)
        throw std::out_of_range("row index exceeds matrix dimensions");
    if(column >= Cols)
        throw std::out_of_range("column index exceeds matrix dimensions");
    return (*this)[row][column];
}

/*!
 * Returns a reference to the element with index (\a row, \a column). A compile time boundary check
 * is performed.
 * This function never fails.
 */
template <uint Rows, uint Cols>
template <uint row, uint column>
double& MatrixBase<Rows, Cols>::get() noexcept
{
    static_assert(row < Rows, "row index must not exceed matrix dimension");
    static_assert(column < Cols, "column index must not exceed matrix dimension");
    return (*this)[row][column];
}
/*!
 * Returns the element with index (\a row, \a column). A compile time boundary check
 * is performed.
 * This function never fails.
 */
template <uint Rows, uint Cols>
template <uint row, uint column>
double MatrixBase<Rows, Cols>::get() const noexcept
{
    static_assert(row < Rows, "row index must not exceed matrix dimension");
    static_assert(column < Cols, "column index must not exceed matrix dimension");
    return (*this)[row][column];
}

/*!
 * Returns a reference to the element from a 1D index lookup \a n, where the matrix elements are
 * stored in row major order. This function is handy in particular when dealing with vectors.
 * A run time boundary check is not performed.
 */
template <uint Rows, uint Cols>
double& MatrixBase<Rows, Cols>::operator()(uint n)
{
    return _m[n];
}
/*!
 * Returns a reference to the element from a 1D index lookup \a n, where the matrix elements are
 * stored in row major order. This function is handy in particular when dealing with vectors.
 * A run time boundary check is not performed.
 */
template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::operator()(uint n) const
{
    return _m[n];
}

/*!
 * Returns a reference to the element from a 1D index lookup \a n, where the matrix elements are
 * stored in row major order. This function is handy in particular when dealing with vectors.
 * A run time boundary check is performed and an exception is thrown if an index exceeds the matrix
 * dimensions (throws out_of_range).
 *
 * \sa at(uint row, uint column)
 */
template <uint Rows, uint Cols>
double& MatrixBase<Rows, Cols>::at(uint n) noexcept(false)
{
    if(n >= Rows * Cols)
        throw std::out_of_range("index exceeds matrix dimensions");
    return _m[n];
}
/*!
 * Returns the element from a 1D index lookup \a n, where the matrix elements are
 * stored in row major order. This function is handy in particular when dealing with vectors.
 * A run time boundary check is performed and an exception is thrown if an index exceeds the matrix
 * dimensions (throws out_of_range).
 *
 * \sa at(uint row, uint column) const
 */
template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::at(uint n) const noexcept(false)
{
    if(n >= Rows * Cols)
        throw std::out_of_range("index exceeds matrix dimensions");
    return _m[n];
}

/*!
 * Returns a reference to the element from a 1D index lookup \a n, where the matrix elements are
 * stored in row major order. This function is handy in particular when dealing with vectors.
 * A compile time boundary check is performed. This function never fails.
 */
template <uint Rows, uint Cols>
template <uint n>
double& MatrixBase<Rows, Cols>::get() noexcept
{
    static_assert(n < Rows * Cols, "index must not exceed matrix dimensions");
    return _m[n];
}
/*!
 * Returns the element from a 1D index lookup \a n, where the matrix elements are
 * stored in row major order. This function is handy in particular when dealing with vectors.
 * A compile time boundary check is performed. This function never fails.
 */
template <uint Rows, uint Cols>
template <uint n>
double MatrixBase<Rows, Cols>::get() const noexcept
{
    static_assert(n < Rows * Cols, "index must not exceed matrix dimensions");
    return _m[n];
}

/*!
 * Returns a pointer to the first element of the matrix.
 * The elements are internally stored in a linear array in row major order.
 * Same as begin().
 */
template <uint Rows, uint Cols>
double* MatrixBase<Rows, Cols>::data() { return _m; }

/*!
 * Returns a pointer to the first element of the matrix. This `const` version prohibits
 * manipulations of the matrix elements through dereferencing the pointer.
 * The elements are internally stored in a linear array in row major order.
 * Same as begin().
 */
template <uint Rows, uint Cols>
const double* MatrixBase<Rows, Cols>::data() const { return _m; }

/*!
 * Returns a pointer to the first element of the matrix. This `const` version prohibits
 * manipulations of the matrix elements through dereferencing the pointer, even when this methods is
 * called from a non-const instance.
 * The elements are internally stored in a linear array in row major order.
 * Same as constBegin().
 */
template <uint Rows, uint Cols>
const double* MatrixBase<Rows, Cols>::constData() const { return _m; }

/*!
 * Returns a pointer to the first element of the matrix.
 * The elements are internally stored in a linear array in row major order.
 * Same as data().
 */
template <uint Rows, uint Cols>
double* MatrixBase<Rows, Cols>::begin() { return _m; }

/*!
 * Returns a pointer to the first element of the matrix. This `const` version prohibits
 * manipulations of the matrix elements through dereferencing the pointer.
 * The elements are internally stored in a linear array in row major order.
 * Same as data().
 */
template <uint Rows, uint Cols>
const double* MatrixBase<Rows, Cols>::begin() const { return _m; }

/*!
 * Returns a pointer to the first element of the matrix. This `const` version prohibits
 * manipulations of the matrix elements through dereferencing the pointer, even when this methods is
 * called from a non-const instance.
 * The elements are internally stored in a linear array in row major order.
 * Same as constData().
 */
template <uint Rows, uint Cols>
const double* MatrixBase<Rows, Cols>::constBegin() const { return _m; }

/*!
 * Returns a pointer to the element following the last element of the matrix.
 * This pointer is useful for having a boundary when looping over the matrix elements.
 * However, attempting to access it results in undefined behavior.
 */
template <uint Rows, uint Cols>
double* MatrixBase<Rows, Cols>::end() { return std::end(_m); }

/*!
 * Returns a pointer to the element following the last element of the matrix.
 * This pointer is useful for having a boundary when looping over the matrix elements.
 * However, attempting to access it results in undefined behavior.
 */
template <uint Rows, uint Cols>
const double* MatrixBase<Rows, Cols>::end() const { return std::end(_m); }

/*!
 * Returns a pointer to the element following the last element of the matrix. This `const` version
 * prohibits manipulations of the matrix elements through dereferencing the pointer, even when this
 * methods is called from a non-const instance.
 * This pointer is useful for having a boundary when looping over the matrix elements.
 * However, attempting to access it results in undefined behavior.
 */
template <uint Rows, uint Cols>
const double* MatrixBase<Rows, Cols>::constEnd() const { return std::end(_m); }

/*!
 * Returns the total number of elements that is `Rows*Cols`.
 */
template <uint Rows, uint Cols>
constexpr size_t MatrixBase<Rows, Cols>::size() const
{
    return static_cast<size_t>(Rows) * static_cast<size_t>(Cols);
}

// formatting matrix entries to a string
/*!
 * Prints the content of the matrix into the returned string. The \a lineModifier may be used to
 * annotate or emphasis the output by prefixing each line by a character sequence.
 *
 * \code
 *  Matrix<2, 4> M{  1.1, -1.2, -1.3, 1.4,
 *                  -2.1,  2.2, -2.3, 2.4 };
 *  std::cout << M.info("important 2 rows: ");
 *
 *  // possible output:
 *  // important 2 rows: |_1.100000___-1.200000___-1.300000____1.400000|
 *  // important 2 rows: |-2.100000____2.200000___-2.300000____2.400000|
 * \endcode
 *
 * The conversion from `double` to string is performed using `std::to_string`.
 * You may switch between language specific decimal "." or "," using the function
 * \code
 *  std::setlocale(LC_NUMERIC, "en_US.UTF-8");
 * \endcode
 * or
 * \code
 *  std::setlocale(LC_NUMERIC, "de_DE.UTF-8");
 * \endcode
 */
template <uint Rows, uint Cols>
std::string MatrixBase<Rows, Cols>::info(const char* lineModifier) const
{
    // number of spaces (SEPARATOR_CHARACTER_FOR_INFO_STRING) between numbers
    // -> must be at least 3, otherwise last column will be truncated
    static const uint numSpacesInBetween = 3;
    // prepare begin of each line (row) using lineModifier
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

/*!
 * Returns the Euclidian norm of a row or column vector,
 * i.e. from a `Matrix<1, N>` or `Matrix<N, 1>`.
 *
 * If the macro `ENABLE_FROBENIUS_NORM` is defined (before `matrix.h`), it can be also used for
 * arbitrary matrices. In this case it computes the Frobenius norm of the matrix (sqrt of the sum
 * of squared elements).
 */
template <uint Rows, uint Cols>
double MatrixBase<Rows, Cols>::norm() const
{
#ifndef ENABLE_FROBENIUS_NORM
    static_assert(Rows == 1 || Cols == 1,
                  "norm is currently only defined for vectors/scalars. "
                  "For using 'norm()' as the Frobenius norm of a matrix, "
                  "please define 'ENABLE_FROBENIUS_NORM' before including 'matrix.h'.");
#endif
    const auto ret = std::inner_product(this->constBegin(), this->constEnd(), this->constBegin(),
                                        0.0);
    return std::sqrt(ret);
}

/*!
 * Returns `true` if all elements are equal, which means they have the equal (byte) representation
 * of all elements; otherwise false.
 */
template <uint Rows, uint Cols>
bool MatrixBase<Rows, Cols>::operator==(const MatrixBase<Rows, Cols>& rhs) const
{
    auto rhsPtr = rhs.begin();

    for(auto val : _m)
    {
        if(val != *rhsPtr)
            return false;
        ++rhsPtr;
    }

    return true;
}

/*!
 * Returns `true` if there is at least one element that is not equal for both matrices, which means
 * the matrices does not have the equal (byte) representation.
 */
template <uint Rows, uint Cols>
bool MatrixBase<Rows, Cols>::operator!=(const MatrixBase<Rows, Cols>& rhs) const
{
    return !(*this == rhs);
}

// ### Matrix template ###
// constructors
/*!
 * Construct an instance and initialize all elements with a \a fillValue.
 *
 * \code
 *  // 4x4 matrix where each element equals one:
 *  Matrix<4, 4> M{ 1.0 };
 * \endcode
 */
template <uint Rows, uint Cols>
Matrix<Rows, Cols>::Matrix(double fillValue)
    : MatrixBase<Rows, Cols>(fillValue)
{
}

/*!
 * Construct an instance from a C-style array. The array length must match the total number of
 * matrix elements, otherwise it results in a compilation error.
 *
 * \code
 *  double ar[4] = { 1., 2., 3., 4. };
    Matrix<2, 2> M{ ar };
 * \endcode
 */
template <uint Rows, uint Cols>
inline Matrix<Rows, Cols>::Matrix(const double (&initArray)[Rows * Cols])
    : MatrixBase<Rows, Cols>(initArray)
{
}

/*!
 * Construct an instance from a list of arguments that specifies each element in row major order.
 * The length of the argument list must match the total number of matrix elements, otherwise it
 * results in a compilation error.
 *
 * \code
 *  Matrix<3, 3> M{ 1.1, 1.2, 1.3,
 *                  2.1, 2.2, 2.3,
 *                  3.1, 3.2, 3.3 };
 * \endcode
 */
template <uint Rows, uint Cols>
template <typename... Doubles, typename>
inline Matrix<Rows, Cols>::Matrix(double firstElement, Doubles... matrixElements)
    : MatrixBase<Rows, Cols>(firstElement, static_cast<double>(matrixElements)...)
{
}

// factory
/*!
 * Returns a \a Rows x \a Cols matrix that is extracted from a \a vector that stores these
 * matrices in row major order. The \a NthMat matrix is read from the container, meaning that
 * \a NthMat*`Rows`*`Cols` elements are skipped.
 * Optionally, a pointer \a ok to a boolean can be passed to check if the reading was successful.
 * It is set to `false` if the \a vector size would be exceeded. In this case it returns a
 * zero-initialized matrix.
 */
template <uint Rows, uint Cols>
template <class Container>
Matrix<Rows, Cols>
Matrix<Rows, Cols>::fromContainer(const Container& vector, size_t NthMat, bool* ok)
{
    auto offSet = NthMat * Rows * Cols;
    if(offSet + Rows * Cols > static_cast<size_t>(vector.size()))
    {
        if(ok) *ok = false;
        return Matrix<Rows, Cols>(0.0);
    }

    Matrix<Rows, Cols> ret;
    auto vecIt = vector.begin() + offSet;
    for(auto& val : ret)
    {
        val = static_cast<double>(*vecIt);
        ++vecIt;
    }

    if(ok) *ok = true;

    return ret;
}

/*!
 * `subMat<fromRow, toRow, fromCol, toCol>()` returns a submatrix specified by the boundary indices
 * for the rows (\a fromRow, \a toRow) and for the columns (\a fromCol, \a toCol).
 * For instance `M.subMat<0,1, 1,1>()` will return the first to elements of the second column of `M`
 * as a 2x1 matrix (vector) if the dimensions of the matrix `M` are at least 2x2, otherwise (in case
 * the dimensions are exceeded) it results in a  compiler error.
 * Except the size of the matrix from which is extracted, there are no further limitation for
 * choosing the boundaries (the template arguments). The dimension of the returned matrix is
 * `abs(toRow-fromRow+1) x abs(toCol-fromCol+1)`.
 * If a 'from' index is larger than a 'to' index, the submatrix is extracted in reverse order, e.g.
 * `M.subMat<1,0, 0,1>()` returns the upper left 2x2 submatrix with its rows flipped in the up-down
 * direction.
 *
 * \code
 *  // Example for extracting a single element from a 2x2 matrix (bottom right corner).
 *  const Matrix<2, 2> mat{ 11., 12.,
 *                          21., 22. };
 *  bool test = (mat.subMat<1,1, 1,1>() == 22.); // test is `true`
 * \endcode
 */
template <uint Rows, uint Cols>
template <uint fromRow, uint toRow, uint fromCol, uint toCol>
auto Matrix<Rows, Cols>::subMat() const ->
Matrix<rangeDim(fromRow, toRow), rangeDim(fromCol, toCol)>
{
    static_assert(fromRow < Rows, "`fromRow` exceeds matrix dimension.");
    static_assert(toRow < Rows, "`toRow` exceeds matrix dimension.");
    static_assert(fromCol < Cols, "`fromCol` exceeds matrix dimension.");
    static_assert(toCol < Cols, "`toCol` exceeds matrix dimension.");

    Matrix<rangeDim(fromRow, toRow), rangeDim(fromCol, toCol)> ret;
    constexpr auto rowInc = toRow >= fromRow ? 1u : static_cast<uint>(-1);
    constexpr auto colInc = toCol >= fromCol ? 1u : static_cast<uint>(-1);

    // suppress MSVS compiler warning `4307` caused by an (intended) integer overflow
#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4307 )
#endif

    for(auto row = fromRow, endRow = toRow + rowInc, subRow = 0u;
        row != endRow;
        row += rowInc, ++subRow)
        for(auto col = fromCol, endCol = toCol + colInc, subCol = 0u;
            col != endCol;
            col += colInc, ++subCol)
            ret(subRow, subCol) = (*this)(row, col);

#ifdef _MSC_VER
    #pragma warning( pop )
#endif

    return ret;
}

/*!
 * This subvector extraction is a simplified overload of `subMat<fromRow, toRow, fromCol, toCol>()`
 * that is more handy to use for vectors. The behavior of `subMat<from, to>()` is equivalent to
 * `subMat<from, to, 0, 0>()` for column vectors or
 * `subMat<0, 0, from, to>()` for row vectors.
 */
template <uint Rows, uint Cols>
template <uint from, uint to>
auto Matrix<Rows, Cols>::subMat() const -> Matrix<vecRowDim(from, to), vecColDim(from, to)>
{
    static_assert(Rows == 1u || Cols == 1u, "`subMat<from, to>()` supports only vectors.");
    constexpr auto nbElem = Rows == 1u ? Cols : Rows;
    static_assert(from < nbElem, "`from` exceeds vector dimension.");
    static_assert(to < nbElem, "`to` exceeds vector dimension.");

    Matrix<vecRowDim(from, to), vecColDim(from, to)> ret;
    constexpr auto inc = to >= from ? 1u : static_cast<uint>(-1);

    // suppress MSVS compiler warning `4307` caused by an (intended) integer overflow
#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4307 )
#endif

    for(auto el = from, endEl = to + inc, sub = 0u;
        el != endEl;
        el += inc, ++sub)
        ret(sub) = (*this)(el);

#ifdef _MSC_VER
    #pragma warning( pop )
#endif

    return ret;
}

/*!
 * Returns the \a i'th row of the matrix. Performes compile time boundary check.
 */
template <uint Rows, uint Cols>
template <uint i>
Matrix<1, Cols> Matrix<Rows, Cols>::row() const
{
    static_assert(i < Rows, "row index must not exceed matrix dimensions");
    Matrix<1, Cols> ret;

    std::copy_n((*this)[i], Cols, ret.begin());

    return ret;
}

/*!
 * Returns the \a j'th column of the matrix. Performes compile time boundary check.
 */
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

/*!
 * Normalizes the current instance of a vector in place by dividing the elements by its Euclidian
 * norm. This transforms the vector into a unit vector.
 * If the macro `ENABLE_FROBENIUS_NORM` has been defined before including the `matrix.h` header
 * this function can be applied for arbitrary matrices (not only vectors). In this case, the matrix
 * is normalized by its Frobenius norm.
 */
template <uint Rows, uint Cols>
void Matrix<Rows, Cols>::normalize()
{
    *this /= this->norm();
}

/*!
 * Returns a (unit) vector (or Matrix if Frobenius norm is enables) that is normalized by its
 * Euclidian norm (or Frobenius norm).
 *
 * To enable the Frobenius norm, `ENABLE_FROBENIUS_NORM` has to be defined before including the
 * `matrix.h` header.
 *
 * \sa normalize()
 */
template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::normalized() const
{
    return *this / this->norm();
}

/*!
 * Returns the transposed matrix.
 */
template <uint Rows, uint Cols>
Matrix<Cols, Rows> Matrix<Rows, Cols>::transposed() const
{
    Matrix<Cols, Rows> ret;
    auto scrPtr = this->constBegin();

    for(auto row = 0u; row < Rows; ++row)
        for(auto column = 0u; column < Cols; ++column)
        {
            ret(column, row) = *scrPtr;
            ++scrPtr;
        }

    return ret;
}

// unary minus operator
template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator-() const
{
    Matrix<Rows, Cols> ret;

    std::transform(this->constBegin(), this->constEnd(), ret.begin(), [](double val) {
        return -val;
    });

    return ret;
}

// compound assignment
template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator*=(double scalar)
{
    std::transform(this->constBegin(), this->constEnd(), this->begin(), [scalar](double val) {
        return val * scalar;
    });

    return *this;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator/=(double scalar)
{
    std::transform(this->constBegin(), this->constEnd(), this->begin(), [scalar](double val) {
        return val / scalar;
    });

    return *this;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator+=(const Matrix<Rows, Cols>& rhs)
{
    std::transform(this->constBegin(), this->constEnd(), rhs.constBegin(), this->begin(),
                   [](double leftVal, double rightVal) {
        return leftVal + rightVal;
    });

    return *this;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols>& Matrix<Rows, Cols>::operator-=(const Matrix<Rows, Cols>& rhs)
{
    std::transform(this->constBegin(), this->constEnd(), rhs.constBegin(), this->begin(),
                   [](double leftVal, double rightVal) {
        return leftVal - rightVal;
    });

    return *this;
}

// binary operators
template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator*(double scalar) const
{
    Matrix<Rows, Cols> ret;

    std::transform(this->constBegin(), this->constEnd(), ret.begin(), [scalar](double val) {
        return val * scalar;
    });

    return ret;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator/(double scalar) const
{
    Matrix<Rows, Cols> ret;

    std::transform(this->constBegin(), this->constEnd(), ret.begin(), [scalar](double val) {
        return val / scalar;
    });

    return ret;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator+(const Matrix<Rows, Cols>& rhs) const
{
    Matrix<Rows, Cols> ret;

    std::transform(this->constBegin(), this->constEnd(), rhs.constBegin(), ret.begin(),
                   std::plus<double>());

    return ret;
}

template <uint Rows, uint Cols>
Matrix<Rows, Cols> Matrix<Rows, Cols>::operator-(const Matrix<Rows, Cols>& rhs) const
{
    Matrix<Rows, Cols> ret;

    std::transform(this->constBegin(), this->constEnd(), rhs.constBegin(), ret.begin(),
                   std::minus<double>());

    return ret;
}

/*!
 * Returns the result of a standard matrix multiplication.
 */
template <uint Rows, uint Cols>
template <uint Cols2>
Matrix<Rows, Cols2>
Matrix<Rows, Cols>::operator*(const Matrix<Cols, Cols2>& rhs) const
{
    const Matrix<Rows, Cols>& lhs = *this;
    Matrix<Rows, Cols2> ret;
    auto retPtr = ret.begin();

    for(auto row = 0u; row < Rows; ++row)
        for(auto column = 0u; column < Cols2; ++column)
        {
            auto val = 0.0;
            auto* lhsPtr = lhs[row];
            auto* rhsPtr = rhs[0] + column;
            for(auto i = 0u; i < Cols; ++i)
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

template <uint Rows, uint Cols>
constexpr uint Matrix<Rows, Cols>::rangeDim(uint from, uint to)
{
    return to >= from ? to - from + 1u : from - to + 1u;
}

template <uint Rows, uint Cols>
constexpr uint Matrix<Rows, Cols>::vecRowDim(uint from, uint to)
{
    return Rows > 1u ? rangeDim(from, to) : 1u;
}

template <uint Rows, uint Cols>
constexpr uint Matrix<Rows, Cols>::vecColDim(uint from, uint to)
{
    return Cols > 1u ? rangeDim(from, to) : 1u;
}

// ### Scalar specialization ###

/*!
 * Initializes the 1x1 matrix with \a value.
 */
inline Matrix<1, 1>::Matrix(double value)
    : MatrixBase<1, 1>(value)
{
}

/*!
 * Returns the value of the 1x1 matrix.
 */
inline double Matrix<1, 1>::value() const { return *begin(); }

/*!
 * Returns a reference to the value of the 1x1 matrix.
 */
inline double& Matrix<1, 1>::ref() { return *begin(); }

/*!
 * Returns a `const` reference to the value of the 1x1 matrix.
 */
inline const double& Matrix<1, 1>::ref() const { return *begin(); }

/*!
 * Implicit conversion to `double`.
 */
inline Matrix<1, 1>::operator double() const { return *begin(); }

inline Matrix<1, 1>& Matrix<1, 1>::operator*=(double scalar)
{
    ref() *= scalar;
    return *this;
}

inline Matrix<1, 1>& Matrix<1, 1>::operator/=(double scalar)
{
    ref() /= scalar;
    return *this;
}

inline Matrix<1, 1>& Matrix<1, 1>::operator+=(double scalar)
{
    ref() += scalar;
    return *this;
}

inline Matrix<1, 1>& Matrix<1, 1>::operator-=(double scalar)
{
    ref() -= scalar;
    return *this;
}

// ### Free functions and operators ###

/*!
* Returns the result of a scalar multiplication of \a scalar with the `Matrix` \a rhs.
*
* \relates Matrix
*/
template <uint Rows, uint Cols>
CTL::mat::Matrix<Rows, Cols> operator*(double scalar, const CTL::mat::Matrix<Rows, Cols>& rhs)
{
    return rhs * scalar;
}

/*!
 * Returns a diagonal \a N x \a N matrix with \a diagElements on the main diagonal.
 *
 * \relates Matrix
 */
template <uint N>
Matrix<N, N> diag(const Matrix<N, 1>& diagElements)
{
    Matrix<N, N> ret(0.0);
    for(uint d = 0; d < N; ++d)
        ret(d, d) = diagElements(d);
    return ret;
}

/*!
 * Returns an \a N x \a N identity matrix.
 *
 * \relates Matrix
 */
template <uint N>
Matrix<N, N> eye()
{
    static Matrix<N, N> ret(0.0);
    static bool toBeInit = true;
    if(toBeInit)
    {
        for(uint i = 0; i < N; ++i)
            ret(i, i) = 1.0;
        toBeInit = false;
    }
    return ret;
}

/*!
 * Returns a (\a Rows) x (\a Cols1 + \a Cols2) matrix that is a horizontal concatenation of
 * \a m1 and \a m2.
 *
 * \relates Matrix
 */
template <uint Rows, uint Cols1, uint Cols2>
Matrix<Rows, Cols1 + Cols2> horzcat(const Matrix<Rows, Cols1>& m1, const Matrix<Rows, Cols2>& m2)
{
    Matrix<Rows, Cols1 + Cols2> ret;
    auto dstPtr = ret.begin();
    for(uint row = 0; row < Rows; ++row)
    {
        std::copy_n(m1[row], Cols1, dstPtr);
        dstPtr += Cols1;
        std::copy_n(m2[row], Cols2, dstPtr);
        dstPtr += Cols2;
    }
    return ret;
}

/*!
 * Returns a (\a Rows1 + \a Rows2) x (\a Cols) matrix that is a vertical concatenation of
 * \a m1 and \a m2.
 *
 * \relates Matrix
 */
template <uint Rows1, uint Rows2, uint Cols>
Matrix<Rows1 + Rows2, Cols> vertcat(const Matrix<Rows1, Cols>& m1, const Matrix<Rows2, Cols>& m2)
{
    Matrix<Rows1 + Rows2, Cols> ret;
    std::copy(m1.begin(), m1.end(), ret[0]);
    std::copy(m2.begin(), m2.end(), ret[Rows1]);
    return ret;
}
// variadic versions (auto return type = C++14 feature)
#if __cplusplus >= 201402L
template <uint Rows, uint Cols1, uint Cols2, class... Matrices>
auto horzcat(const Matrix<Rows, Cols1>& m1, const Matrix<Rows, Cols2>& m2, const Matrices&... mats)
{
    return horzcat(horzcat(m1, m2), mats...);
}
template <uint Rows1, uint Rows2, uint Cols, class... Matrices>
auto vertcat(const Matrix<Rows1, Cols>& m1, const Matrix<Rows2, Cols>& m2, const Matrices&... mats)
{
    return vertcat(vertcat(m1, m2), mats...);
}
#endif

} // namespace mat
} // namespace CTL
