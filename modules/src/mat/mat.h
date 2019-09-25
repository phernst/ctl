#ifndef MAT_H
#define MAT_H

// This header includes everything from the `CTL::mat` namespace.
// Plus, it defines a user-defined floating-point literal for conversion from radian to degree.

#include "matrix_algorithm.h"
#include "pmatcomparator.h"

constexpr long double operator"" _deg(long double deg)
{
    return deg * 3.141592653589793238462643383279502884l / 180.0l;
}

#endif // MAT_H
