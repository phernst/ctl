#ifndef MAT_H
#define MAT_H

// header that includes everything from the `CTL::mat` namespace
// + Matrix template class (+ MatrixBase + typedefs in "matrix_types.h")

#include "matrix_algorithm.h"
#include "pmatcomparator.h"

constexpr long double operator"" _deg(long double deg)
{
#ifdef M_PIl
    return deg * M_PIl / 180.0l;
#else
    return deg * static_cast<long double>(M_PI) / 180.0l;
#endif
}

#endif // MAT_H
