#ifndef FULLGEOMETRY_H
#define FULLGEOMETRY_H

#include "mat/projectionmatrix.h"
#include <QVector>

namespace CTL {

/*!
 * \brief Alias name for a vector of projection matrices.
 * \relates GeometryEncoder
 * \relates GeometryDecoder
 *
 * The individual projection matrices in a SingleViewGeometry usually correspond to the individual
 * detector modules of the detector system. Each of the modules is a flat panel whose geometry is
 * described by one ProjectionMatrix.
 *
 * Placeholder; to be replaced by dedicated class later.
 */
class SingleViewGeometry
{
public:
    SingleViewGeometry() = default;
    explicit SingleViewGeometry(uint nbModules);
    explicit SingleViewGeometry(QVector<mat::ProjectionMatrix>&& pMats);
    explicit SingleViewGeometry(const QVector<mat::ProjectionMatrix>& pMats);

    const mat::ProjectionMatrix& at(uint i) const;
    const mat::ProjectionMatrix& module(uint i) const;
    const mat::ProjectionMatrix& first() const;
    uint length() const;
    uint nbModules() const;
    uint size() const;

    QVector<mat::ProjectionMatrix>::iterator begin();
    QVector<mat::ProjectionMatrix>::const_iterator begin() const;

    QVector<mat::ProjectionMatrix>::iterator end();
    QVector<mat::ProjectionMatrix>::const_iterator end() const;

    const mat::ProjectionMatrix& operator[](uint i) const;
    mat::ProjectionMatrix& operator[](uint i);

    void append(const mat::ProjectionMatrix& pMat);
    void append(const QVector<mat::ProjectionMatrix>& pMats);
    void append(const SingleViewGeometry& other);
    void clear();
    void reserve(uint nbModules);

    typedef typename QVector<mat::ProjectionMatrix>::const_iterator const_iterator;

private:
    QVector<mat::ProjectionMatrix> _pMats;
};

/*!
 * \brief Alias name for a vector of SingleViewGeometry elements.
 * \relates GeometryEncoder
 * \relates GeometryDecoder
 *
 * This is used to store the geometry (encoded in projection matrices) for multiple views. Whereas
 * all projection matrices in a SingleViewGeometry correspond to identical system settings (e.g.
 * gantry position etc.), settings may differ from view to view (i.e. in different elements of
 * FullGeometry).
 *
 * Placeholder; to be replaced by dedicated class later.
 */
typedef QVector<SingleViewGeometry> FullGeometry;

inline SingleViewGeometry::SingleViewGeometry(uint nbModules)
    : _pMats(static_cast<int>(nbModules))
{
}

inline SingleViewGeometry::SingleViewGeometry(QVector<mat::ProjectionMatrix>&& pMats)
    : _pMats(std::move(pMats))
{
}

inline SingleViewGeometry::SingleViewGeometry(const QVector<mat::ProjectionMatrix>& pMats)
    : _pMats(pMats)
{
}

inline const mat::ProjectionMatrix& SingleViewGeometry::at(uint i) const
{
    return _pMats.at(static_cast<int>(i));
}

inline const mat::ProjectionMatrix& SingleViewGeometry::module(uint i) const
{
    return this->at(i);
}

inline const mat::ProjectionMatrix& SingleViewGeometry::first() const
{
    return _pMats.first();
}

inline uint SingleViewGeometry::length() const
{
    return this->size();
}

inline uint SingleViewGeometry::nbModules() const
{
    return this->size();
}

inline uint SingleViewGeometry::size() const
{
    return static_cast<uint>(_pMats.size());
}

inline QVector<mat::ProjectionMatrix>::iterator SingleViewGeometry::begin()
{
    return _pMats.begin();
}

inline QVector<mat::ProjectionMatrix>::const_iterator SingleViewGeometry::begin() const
{
    return _pMats.begin();
}

inline QVector<mat::ProjectionMatrix>::iterator SingleViewGeometry::end()
{
    return _pMats.end();
}

inline QVector<mat::ProjectionMatrix>::const_iterator SingleViewGeometry::end() const
{
    return _pMats.end();
}

inline const mat::ProjectionMatrix& SingleViewGeometry::operator[](uint i) const
{
    return this->at(i);
}

inline mat::ProjectionMatrix& SingleViewGeometry::operator[](uint i)
{
    return _pMats[static_cast<int>(i)];
}

inline void SingleViewGeometry::append(const mat::ProjectionMatrix& pMat)
{
    _pMats.append(pMat);
}

inline void SingleViewGeometry::append(const QVector<mat::ProjectionMatrix> &pMats)
{
    _pMats.append(pMats);
}

inline void SingleViewGeometry::append(const SingleViewGeometry& other)
{
    _pMats.append(other._pMats);
}

inline void SingleViewGeometry::clear()
{
    _pMats.clear();
}

inline void SingleViewGeometry::reserve(uint nbModules)
{
    _pMats.reserve(static_cast<int>(nbModules));
}

} // namespace CTL

/*! \file */
///@{

///@}
///
#endif // FULLGEOMETRY_H
