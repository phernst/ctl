#ifndef FULLGEOMETRY_H
#define FULLGEOMETRY_H

#include "mat/projectionmatrix.h"
#include <QVector>

namespace CTL {

typedef QVector<mat::ProjectionMatrix> SingleViewGeometry;
typedef QVector<SingleViewGeometry> FullGeometry;

} // namespace CTL

/*! \file */
///@{
/*!
 * \typedef typedef QVector<mat::ProjectionMatrix> CTL::SingleViewGeometry;
 * \brief Alias name for a vector of projection matrices.
 * \relates CTL::GeometryEncoder
 * \relates CTL::GeometryDecoder
 *
 * The individual projection matrices in a SingleViewGeometry usually correspond to the individual
 * detector modules of the detector system. Each of the modules is a flat panel whose geometry is
 * described by one ProjectionMatrix.
 */

/*!
 * \typedef typedef QVector<SingleViewGeometry> CTL::FullGeometry;
 * \brief Alias name for a vector of SingleViewGeometry elements.
 * \relates CTL::GeometryEncoder
 * \relates CTL::GeometryDecoder
 *
 * This is used to store the geometry (encoded in projection matrices) for multiple views. Whereas
 * all projection matrices in a SingleViewGeometry correspond to identical system settings (e.g.
 * gantry position etc.), settings may differ from view to view (i.e. in different elements of
 * FullGeometry).
 */
///@}
///
#endif // FULLGEOMETRY_H
