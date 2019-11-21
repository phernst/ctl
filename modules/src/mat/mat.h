#ifndef MAT_H
#define MAT_H

/*
 * This is a wrapping header, which includes everything from the `CTL::mat` namespace.
 * Plus it defines a user-defined floating-point literal `deg` for conversion from degree to radian.
 *
 * The following types will be directly defined within the `CTL` namespace (see matrix_types.h),
 * i.e. not encapsulated in `CTL::mat`:
 * `ProjectionMatrix`, `Homography2D`, `Homography3D`, `Matrix3x3`, `Vector3x1`.
 */

#include "matrix_algorithm.h"
#include "pmatcomparator.h"
#include "deg.h"

#endif // MAT_H
