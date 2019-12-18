/******************************************************************************
** 'Matrix' template class for basic matrix calculations
** by Robert Frysch | Dec 18, 2019
** Otto von Guericke University Magdeburg
** Institute for Medical Engineering - IMT (Head: Georg Rose)
** Email: robert.frysch@ovgu.de
******************************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include <algorithm>
#include <cmath> // required for sqrt() in 'MatrixBase::norm()'
#include <string>

typedef unsigned int uint;

namespace CTL {
namespace mat {

/*!
 * \class MatrixBase
 *
 * \brief Helper base class that provides an access interface to the Matrix template class and its
 * scalar specialization.
 */

/*!
 * \class Matrix
 *
 * \brief This template class is an abstraction of a small matrix with a size known at compile time.
 *
 * Basic algebraic operations are provided and dimension checks are carried out during compilation.
 * No heap allocation is performed.
 * Elements are stored in row major order.
 */

// uniform interface and ressource
template <uint Rows, uint Cols>
class MatrixBase
{
public:
    // construction
    MatrixBase() = default;
    explicit MatrixBase(double fillValue);
    explicit MatrixBase(const double (&initArray)[Rows * Cols]);

    // select row
    double* operator[](uint row);
    const double* operator[](uint row) const;

    // individual element access with 2 indizes
    // -> standard access (without boundary check)
    double& operator()(uint row, uint column);
    double operator()(uint row, uint column) const;
    // -> run time boundary check (throws out_of_range)
    double& at(uint row, uint column) noexcept(false);
    double at(uint row, uint column) const noexcept(false);
    // -> compile time boundary check (never fails)
    template <uint row, uint column>
    double& get() noexcept;
    template <uint row, uint column>
    double get() const noexcept;

    // individual element access with 1 index
    // -> standard access (without boundary check)
    double& operator()(uint n);
    double operator()(uint n) const;
    // -> run time boundary check (throws out_of_range)
    double& at(uint n);
    double at(uint n) const;
    // -> compile time boundary check (never fails)
    template <uint n>
    double& get() noexcept;
    template <uint n>
    double get() const noexcept;

    // pointer access to array (row-major order)
    double* data() { return _m; } // optional, for convenience
    const double* data() const { return _m; } // optional, for convenience
    const double* constData() const { return _m; } // optional, for convenience
    double* begin() { return _m; }
    const double* begin() const { return _m; }
    const double* constBegin() const { return _m; }
    double* end() { return std::end(_m); }
    const double* end() const { return std::end(_m); }
    const double* constEnd() const { return std::end(_m); }

    // size
    constexpr size_t size() const;

    // convert content to string
    static const char SEPARATOR_CHARACTER_FOR_INFO_STRING = '_';
    std::string info(const char* lineModifier = "") const;

    // Euclidean norm of a vector or absolute value of a scalar
    double norm() const;

    // equality operator
    bool operator==(const MatrixBase<Rows, Cols>& other) const;
    bool operator!=(const MatrixBase<Rows, Cols>& other) const;

private:
    // the data
    double _m[Rows * Cols];
};

// actual Matrix class for matrices and vectors
template <uint Rows, uint Cols>
class Matrix : public MatrixBase<Rows, Cols>
{
    // helper function for `subMat()`
    static constexpr uint rangeDim(uint from, uint to);
    // helper function for vectors' `subMat()`
    static constexpr uint vecRowDim(uint from, uint to);
    static constexpr uint vecColDim(uint from, uint to);

public:
    Matrix() = default;
    explicit Matrix(double fillValue);
    explicit Matrix(const double (&initArray)[Rows * Cols]);
    template <typename... Doubles,
              typename = typename std::enable_if<sizeof...(Doubles) + 1u == Rows * Cols>::type>
    Matrix(double firstElement, Doubles... matrixElements);

    // factory function that copies (+ cast if necessary) the 'NthMat' matrix from
    // a Container (stack of matrices)
    template <class Container>
    static Matrix<Rows, Cols>
    fromContainer(const Container& vector, size_t NthMat, bool* ok = nullptr);

    // sub-matrix extraction
    template <uint fromRow, uint toRow, uint fromCol, uint toCol>
    auto subMat() const -> Matrix<rangeDim(fromRow, toRow), rangeDim(fromCol, toCol)>;
    // sub-vector extraction
    template <uint from, uint to>
    auto subMat() const -> Matrix<vecRowDim(from, to), vecColDim(from, to)>;

    // single vector extraction
    template <uint i>
    Matrix<1, Cols> row() const;
    template <uint j>
    Matrix<Rows, 1> column() const;

    // unary operators
    Matrix<Cols, Rows> transposed() const;
    Matrix<Rows, Cols> operator-() const;
    // compound assignment
    Matrix<Rows, Cols>& operator*=(double scalar);
    Matrix<Rows, Cols>& operator/=(double scalar);
    Matrix<Rows, Cols>& operator+=(const Matrix<Rows, Cols>& other);
    Matrix<Rows, Cols>& operator-=(const Matrix<Rows, Cols>& other);
    // binary operators
    Matrix<Rows, Cols> operator*(double scalar) const;
    Matrix<Rows, Cols> operator/(double scalar) const;
    Matrix<Rows, Cols> operator+(const Matrix<Rows, Cols>& rhs) const;
    Matrix<Rows, Cols> operator-(const Matrix<Rows, Cols>& rhs) const;
    template <uint Cols2> // standard matrix multiplication
    Matrix<Rows, Cols2> operator*(const Matrix<Cols, Cols2>& rhs) const;
};

// scalar specialization
template <>
class Matrix<1, 1> : public MatrixBase<1, 1>
{
public:
    Matrix() = default;
    Matrix(double value)
        : MatrixBase<1, 1>(value)
    {
    }

    // dedicated access to scalar value
    double value() const { return *begin(); }
    double& ref() { return *begin(); }
    const double& ref() const { return *begin(); }

    // implicit conversion to 'double'
    operator double() const { return *begin(); }

    // operations
    Matrix<1, 1> transposed() const { return *this; }
    Matrix<1, 1>& operator*=(double scalar)
    {
        ref() *= scalar;
        return *this;
    }
    Matrix<1, 1>& operator/=(double scalar)
    {
        ref() /= scalar;
        return *this;
    }
    Matrix<1, 1>& operator+=(double scalar)
    {
        ref() += scalar;
        return *this;
    }
    Matrix<1, 1>& operator-=(double scalar)
    {
        ref() -= scalar;
        return *this;
    }
};

} // namespace mat
} // namespace CTL

// global operators
template <uint Rows, uint Cols>
CTL::mat::Matrix<Rows, Cols> operator*(double scalar, const CTL::mat::Matrix<Rows, Cols>& rhs)
{
    return rhs * scalar;
}

#include "matrix.tpp"

#endif // MATRIX_H
