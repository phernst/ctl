#include "viewgeometry.h"

namespace CTL {

SingleViewGeometry::SingleViewGeometry(uint nbModules)
    : _pMats(static_cast<int>(nbModules))
{
}

SingleViewGeometry::SingleViewGeometry(QVector<mat::ProjectionMatrix>&& pMats)
    : _pMats(std::move(pMats))
{
}

SingleViewGeometry::SingleViewGeometry(const QVector<mat::ProjectionMatrix>& pMats)
    : _pMats(pMats)
{
}

const mat::ProjectionMatrix& SingleViewGeometry::at(uint i) const { return _pMats.at(static_cast<int>(i)); }

const mat::ProjectionMatrix& SingleViewGeometry::module(uint i) const { return this->at(i); }

const mat::ProjectionMatrix& SingleViewGeometry::first() const { return _pMats.first(); }

uint SingleViewGeometry::length() const { return this->size(); }

uint SingleViewGeometry::nbModules() const { return this->size(); }

uint SingleViewGeometry::size() const { return static_cast<uint>(_pMats.size()); }

SingleViewGeometry::iterator SingleViewGeometry::begin() { return _pMats.begin(); }

SingleViewGeometry::const_iterator SingleViewGeometry::begin() const { return _pMats.begin(); }

SingleViewGeometry::iterator SingleViewGeometry::end() { return _pMats.end(); }

SingleViewGeometry::const_iterator SingleViewGeometry::end() const { return _pMats.end(); }

const mat::ProjectionMatrix& SingleViewGeometry::operator[](uint i) const { return this->at(i); }

mat::ProjectionMatrix& SingleViewGeometry::operator[](uint i) { return _pMats[static_cast<int>(i)]; }

void SingleViewGeometry::append(const mat::ProjectionMatrix& pMat) { _pMats.append(pMat); }

void SingleViewGeometry::append(const QVector<mat::ProjectionMatrix>& pMats) { _pMats.append(pMats); }

void SingleViewGeometry::append(const SingleViewGeometry& other) { _pMats.append(other._pMats); }

void SingleViewGeometry::clear() { _pMats.clear(); }

std::vector<float> SingleViewGeometry::concatenatedStdVector() const
{
    std::vector<float> ret;
    ret.reserve(_pMats.size() * 12);

    for(const auto& pmat : qAsConst(_pMats))
        for(const auto el : pmat)
            ret.push_back(static_cast<float>(el));

    return ret;
}

void SingleViewGeometry::reserve(uint nbModules) { _pMats.reserve(static_cast<int>(nbModules)); }

FullGeometry::FullGeometry(uint nbViews)
    : _viewGeos(nbViews)
{
}

FullGeometry::FullGeometry(QVector<SingleViewGeometry>&& pMats)
    : _viewGeos(std::move(pMats))
{
}

FullGeometry::FullGeometry(const QVector<SingleViewGeometry>& pMats)
    : _viewGeos(pMats)
{
}

const SingleViewGeometry& FullGeometry::at(uint i) const { return _viewGeos.at(static_cast<int>(i)); }

const SingleViewGeometry& FullGeometry::view(uint i) const { return this->at(i); }

const SingleViewGeometry& FullGeometry::first() const { return _viewGeos.first(); }

uint FullGeometry::length() const { return this->size(); }

uint FullGeometry::nbViews() const { return this->size(); }

uint FullGeometry::size() const { return static_cast<uint>(_viewGeos.size()); }

FullGeometry::iterator FullGeometry::begin() { return _viewGeos.begin(); }

FullGeometry::const_iterator FullGeometry::begin() const { return _viewGeos.begin(); }

FullGeometry::iterator FullGeometry::end() { return _viewGeos.end(); }

FullGeometry::const_iterator FullGeometry::end() const { return _viewGeos.end(); }

const SingleViewGeometry& FullGeometry::operator[](uint i) const { return this->at(i); }

SingleViewGeometry& FullGeometry::operator[](uint i) { return _viewGeos[static_cast<int>(i)]; }

void FullGeometry::append(const SingleViewGeometry &view) { _viewGeos.append(view); }

void FullGeometry::append(const FullGeometry &other) { _viewGeos.append(other._viewGeos); }

void FullGeometry::clear() { _viewGeos.clear(); }

void FullGeometry::reserve(uint nbViews) { _viewGeos.reserve(static_cast<int>(nbViews)); }

std::vector<float> FullGeometry::concatenatedStdVector() const
{
    std::vector<float> ret;

    const auto nbViews = this->nbViews();
    if(nbViews == 0)
        return ret;

    ret.reserve(nbViews * this->view(0).nbModules() * 12);

    for(const auto& view : *this)
        for(const auto viewPMat : view.concatenatedStdVector())
            ret.push_back(viewPMat);

    return ret;
}

} // namespace CTL
