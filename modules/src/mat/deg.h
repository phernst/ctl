#ifndef CTL_DEG_H
#define CTL_DEG_H

#include "pi.h"

/*!
 * Defines a user-defined floating-point literal for conversion from degree to radian.
 * Usage: `90.0_deg` results in a (long) double value of pi/2 = 1.57... .
 */
constexpr long double operator"" _deg(long double deg)
{
    return deg * PIl / 180.0l;
}

#endif // CTL_DEG_H
