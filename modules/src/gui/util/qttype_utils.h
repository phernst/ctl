#ifndef QTTYPE_UTILS_H
#define QTTYPE_UTILS_H

#include "mat/matrix_types.h"
#include <QQuaternion>

/*
 * NOTE: This is header only.
 */

// free global function declaration
inline QVector3D toQVector3D(const CTL::Vector3x1& vector);
inline QMatrix3x3 toQMatrix3x3(const CTL::Matrix3x3& matrix);
inline QQuaternion toQQuaternion(const CTL::Matrix3x3& matrix);

inline QVector3D toQVector3D(const CTL::Vector3x1 & vector)
{
    return { float(vector.get<0>()), float(vector.get<1>()), float(vector.get<2>()) };
}

inline QMatrix3x3 toQMatrix3x3(const CTL::Matrix3x3& matrix)
{
    float tmp[9];
    float* ptr = tmp;
    for(auto val : matrix)
        *ptr++ = float(val);
    return QMatrix3x3(tmp);
}

inline QQuaternion toQQuaternion(const CTL::Matrix3x3& matrix)
{
    return QQuaternion::fromRotationMatrix(toQMatrix3x3(matrix));
}

#endif // QTTYPE_UTILS_H
