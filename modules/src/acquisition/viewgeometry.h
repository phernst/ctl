#ifndef CTL_VIEWGEOMETRY_H
#define CTL_VIEWGEOMETRY_H

#include "mat/projectionmatrix.h"
#include <QVector>

namespace CTL {

/*!
 * \class SingleViewGeometry
 *
 * \brief Holds a list of projection matrices corresponding to the detector modules of a single
 * view.
 * \relates GeometryEncoder
 * \relates GeometryDecoder
 *
 * The individual projection matrices in a SingleViewGeometry usually correspond to the individual
 * detector modules of the detector system. Each of the modules is a flat panel whose geometry is
 * described by one ProjectionMatrix.
 */
class SingleViewGeometry
{
public:
    typedef typename QVector<mat::ProjectionMatrix>::iterator iterator;
    typedef typename QVector<mat::ProjectionMatrix>::const_iterator const_iterator;

    SingleViewGeometry() = default;
    explicit SingleViewGeometry(uint nbModules);
    explicit SingleViewGeometry(QVector<mat::ProjectionMatrix>&& pMats);
    explicit SingleViewGeometry(const QVector<mat::ProjectionMatrix>& pMats);

    void append(const mat::ProjectionMatrix& pMat);
    void append(const QVector<mat::ProjectionMatrix>& pMats);
    void append(const SingleViewGeometry& other);
    const mat::ProjectionMatrix& at(uint i) const;
    iterator begin();
    const_iterator begin() const;
    void clear();
    std::vector<float> concatenatedStdVector() const;
    iterator end();
    const_iterator end() const;
    const mat::ProjectionMatrix& first() const;
    uint length() const;
    const mat::ProjectionMatrix& module(uint i) const;
    uint nbModules() const;
    void reserve(uint nbModules);
    uint size() const;

    const mat::ProjectionMatrix& operator[](uint i) const;
    mat::ProjectionMatrix& operator[](uint i);

private:
    QVector<mat::ProjectionMatrix> _pMats;
};

/*!  
 * \class FullGeometry
 *
 * \brief Holds a list of SingleViewGeometry instances to represent the acquisition geometry of a
 * full CT scan.
 * \relates GeometryEncoder
 * \relates GeometryDecoder
 *
 * This is used to store the geometry (encoded in projection matrices) for multiple views. Whereas
 * all projection matrices in a SingleViewGeometry correspond to identical system settings (e.g.
 * gantry position etc.), settings may differ from view to view (i.e. in different elements of
 * FullGeometry).
 */
class FullGeometry
{
public:
    typedef typename QVector<SingleViewGeometry>::iterator iterator;
    typedef typename QVector<SingleViewGeometry>::const_iterator const_iterator;

    FullGeometry() = default;
    explicit FullGeometry(uint nbViews);
    explicit FullGeometry(QVector<SingleViewGeometry>&& pMats);
    explicit FullGeometry(const QVector<SingleViewGeometry>& pMats);

    void append(const SingleViewGeometry& view);
    void append(const QVector<mat::ProjectionMatrix>& pMats);
    void append(const FullGeometry& other);
    const SingleViewGeometry& at(uint i) const;
    iterator begin();
    const_iterator begin() const;
    void clear();
    std::vector<float> concatenatedStdVector() const;
    iterator end();
    const_iterator end() const;
    const SingleViewGeometry& first() const;
    uint length() const;
    uint nbViews() const;
    void reserve(uint nbViews);
    uint size() const;
    const SingleViewGeometry& view(uint i) const;

    const SingleViewGeometry& operator[](uint i) const;
    SingleViewGeometry& operator[](uint i);

private:
    QVector<SingleViewGeometry> _viewGeos;
};

} // namespace CTL

/*! \file */
///@{

///@}
///
#endif // CTL_VIEWGEOMETRY_H
