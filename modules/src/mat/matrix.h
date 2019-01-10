/******************************************************************************
** 'Matrix' template class for basic matrix calculations
** by Robert Frysch | June 05, 2018
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

// uniform interface and ressource
template <uint Rows, uint Cols>
class MatrixBase
{
public:
    // construction
    MatrixBase() = default;
    explicit MatrixBase(double fillValue);
    MatrixBase(const double (&initArray)[Rows * Cols]);

    // select row
    double* operator[](uint row) { return _m + row * Cols; }
    const double* operator[](uint row) const { return _m + row * Cols; }

    // individual element access with 2 indizes
    // -> standard access (without boundary check)
    double& operator()(uint row, uint column) { return (*this)[row][column]; }
    double operator()(uint row, uint column) const { return (*this)[row][column]; }
    // -> run time boundary check (throws out_of_range)
    double& at(uint row, uint column);
    double at(uint row, uint column) const;
    // -> compile time boundary check (never fails)
    template <uint row, uint column>
    double& get() noexcept;
    template <uint row, uint column>
    double get() const noexcept;

    // individual element access with 1 index
    // -> standard access (without boundary check)
    double& operator()(uint n) { return _m[n]; }
    double operator()(uint n) const { return _m[n]; }
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
public:
    Matrix() = default;
    explicit Matrix(double fillValue)
        : MatrixBase<Rows, Cols>(fillValue)
    {
    }
    Matrix(const double (&initArray)[Rows * Cols])
        : MatrixBase<Rows, Cols>(initArray)
    {
    }

    // factory function that copies (+ cast if necessary) the 'NthMat' matrix from
    // a Container (stack of matrices)
    template <class Container>
    static Matrix<Rows, Cols>
    fromContainer(const Container& vector, size_t NthMat, bool* ok = nullptr);

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
