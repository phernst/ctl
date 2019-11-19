#ifndef MAT_H
#define MAT_H

// This header includes everything from the `CTL::mat` namespace.
// Plus, it defines a user-defined floating-point literal for conversion from radian to degree.

#include "matrix_algorithm.h"
#include "pmatcomparator.h"

constexpr const long double PIl = 3.141592653589793238462643383279502884L; // pi
constexpr const double PI       = 3.14159265358979323846; // pi
constexpr const double PI_2     = 1.57079632679489661923; // pi/2
constexpr const double PI_4     = 0.78539816339744830962; // pi/4

constexpr long double operator"" _deg(long double deg)
{
    return deg * PIl / 180.0l;
}

#endif // MAT_H
