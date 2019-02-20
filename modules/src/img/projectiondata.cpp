#include "projectiondata.h"
#include <future>

namespace CTL
{
/*!
 * Constructs a ProjectionData object with dimensions for the individual single views as specified
 * by \a viewDimensions. This does not allocate any memory for the actual data. To (explicitely)
 * do so, use allocateMemory().
 */
ProjectionData::ProjectionData(const SingleViewData::Dimensions &viewDimensions)
    : _viewDim(viewDimensions)
{

}
/*!
 * Constructs a ProjectionData object with dimensions for the individual single views specified
 * by \a channelsPerModule, \a rowsPerModule and \a nbModules. This does not allocate any memory
 * for the actual data. To (explicitely) do so, use allocateMemory().
 */
ProjectionData::ProjectionData(uint channelsPerModule, uint rowsPerModule, uint nbModules)
    : _viewDim({channelsPerModule, rowsPerModule, nbModules})
{

}

/*!
 * Appends the data from \a singleView to this single view. The dimensions of \a singleView must
 * match the dimensions specified for single views in this dataset. Throws std::domain_error in
 * case of mismatching dimensions.
 *
 * Overloaded method that uses move-semantics. \sa append(const SingleViewData &)
 */
void ProjectionData::append(SingleViewData &&singleView)
{
    if(!hasEqualSizeAs(singleView))
        throw std::domain_error("SingleViewData has incompatible size for ProjectionData:\n[" +
                                _viewDim.info() + "].append([" + singleView.dimensions().info() + "])");

    _data.push_back(std::move(singleView));
}

/*!
 * Appends the data from \a singleView to this single view. The dimensions of \a singleView must
 * match the dimensions specified for single views in this dataset. Throws std::domain_error in
 * case of mismatching dimensions.
 *
 * \sa append(SingleViewData&&)
 */
void ProjectionData::append(const SingleViewData &singleView)
{
    if(!hasEqualSizeAs(singleView))
        throw std::domain_error("SingleViewData has incompatible size for ProjectionData:\n[" +
                                _viewDim.info() + "].append([" + singleView.dimensions().info() + "])");

    _data.push_back(singleView);
}

/*!
 * Returns the maximum value in this instance.
 *
 * Returns zero if this data is empty.
 */
float ProjectionData::max() const
{
    if(nbViews() == 0)
        return 0.0f;

    float tmpMax = view(0).max();

    float locMax;
    for(int vi = 1, total = nbViews(); vi < total; ++vi)
    {
        locMax = view(vi).max();
        if(locMax > tmpMax)
            tmpMax = locMax;
    }

    return tmpMax;
}

/*!
 * Returns the minimum value in this instance.
 *
 * Returns zero if this data is empty.
 */
float ProjectionData::min() const
{
    if(nbViews() == 0)
        return 0.0f;

    float tmpMin = view(0).min();

    float locMin;
    for(int vi = 1, total = nbViews(); vi < total; ++vi)
    {
        locMin = view(vi).min();
        if(locMin < tmpMin)
            tmpMin = locMin;
    }

    return tmpMin;
}

/*!
 * Sets the projection data of this instance based on the data given by \a dataVector. For this, it
 * is assumed that the data in \a dataVector has row major order, i.e. all values of a module row,
 * followed by the remaining rows, the other modules of the same view and finally all other views.
 *
 * This method is provided for convenience to serve as an alternative to use append() for individual
 * views.
 */
void ProjectionData::setDataFromVector(const std::vector<float> &dataVector)
{
    const size_t elementsPerView = _viewDim.nbChannels*_viewDim.nbRows*_viewDim.nbModules;

    Q_ASSERT(elementsPerView);
    if(elementsPerView == 0)
        throw std::domain_error("ProjectionData has null-dimension");

    // check if number of elements in 'dataVector' is multiple of required number of elements in a single view (specified in '_viewDim')
    Q_ASSERT(dataVector.size() % elementsPerView == 0);
    if(dataVector.size() % elementsPerView != 0)
        throw std::domain_error("data vector has incompatible size for ProjectionData");

    // start building new data
    _data.clear(); // clear all previous data

    const size_t nbViewsToBuild = dataVector.size() / elementsPerView;

    for(size_t view=0; view<nbViewsToBuild; ++view)
    {
        SingleViewData singleView(_viewDim.nbChannels, _viewDim.nbRows);

        std::vector<float> tmp(elementsPerView);
        std::copy_n(dataVector.data() + view*elementsPerView, elementsPerView, tmp.data());
        singleView.setDataFromVector(tmp);

        _data.push_back(std::move(singleView));
    }
}

/*!
 * Combines the projection data from all modules into single Chunk2Ds for all views and returns the
 * result.
 *
 * To combine the data, a \a layout is required that describes the arrangement of the individual
 * modules.
 */
ProjectionData ProjectionData::combined(const ModuleLayout& layout) const
{
    if(layout.isEmpty())
        return combined(ModuleLayout::canonicLayout(1, std::max(_viewDim.nbModules, 1u)));

    SingleViewData::ModuleData::Dimensions moduleDim{ _viewDim.nbChannels * layout.columns(),
                                                      _viewDim.nbRows * layout.rows() };
    ProjectionData ret(moduleDim.width, moduleDim.height, 1);

    for(uint view = 0, nbViews = this->nbViews(); view < nbViews; ++view)
    {
        SingleViewData viewData(moduleDim);
        viewData.append(_data[view].combined(layout));
        ret.append(std::move(viewData));
    }

    return ret;
}

/*!
 * Concatenates the projection data from all views and returns it as a one-dimensional vector.
 */
std::vector<float> ProjectionData::toVector() const
{
    auto dim = dimensions();
    const auto viewSize = dim.nbChannels * dim.nbRows * dim.nbModules;

    std::vector<float> ret;
    ret.resize(dim.totalNbElements());

    auto destIt = ret.begin();

    for(size_t view=0; view<dim.nbViews; ++view)
    {
        auto viewVectorized = _data[view].toVector();
        auto srcIt = viewVectorized.cbegin();
        std::copy_n(srcIt, viewSize, destIt);
        destIt += viewSize;
    }

    return ret;
}

/*!
 * Transforms all data values in this instance to extinction (w.r.t. the initial intensity passed
 * by \a i0) using the following formula:
 *
 * \f$
 * \mathtt{newValue}=\ln\frac{i0}{\mathtt{oldValue}}.
 * \f$
 */
void ProjectionData::transformToExtinction(double i0)
{
    std::vector<std::future<void>> futures;
    futures.reserve(nbViews());

    for(auto& singleView : _data)
        futures.push_back(std::async(&SingleViewData::transformToExtinction,
                                     &singleView,
                                     i0));
}

/*!
 * Transforms all data values in this instance to intensities (w.r.t. the initial intensity passed
 * by \a i0) using the following formula:
 *
 * \f$
 * \mathtt{newValue}=i0\cdot\exp(-\mathtt{oldValue}).
 * \f$
 */
void ProjectionData::transformToIntensity(double i0)
{
    std::vector<std::future<void>> futures;
    futures.reserve(nbViews());

    for(auto& singleView : _data)
        futures.push_back(std::async(&SingleViewData::transformToIntensity,
                                     &singleView,
                                     i0));
}

/*!
 * Returns true if the dimensions of \a other are equal to those of the views in this instance.
 */
bool ProjectionData::hasEqualSizeAs(const SingleViewData &other) const
{
    Q_ASSERT(other.dimensions() == _viewDim);
    return other.dimensions() == _viewDim;
}

/*!
 * Enforces memory allocation and allocates memory for \a nbViews views.
 * As a result, the number of views is equal to \a nbViews.
 *
 * \sa SingleViewData::allocateMemory().
 */
void ProjectionData::allocateMemory(uint nbViews)
{
    _data.resize(nbViews, SingleViewData(_viewDim.nbChannels,_viewDim.nbRows));

    for(auto& view : _data)
        view.allocateMemory(_viewDim.nbModules);
}

/*!
 * Adds the data from \a other to this instance and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this instance do not match.
 */
ProjectionData& ProjectionData::operator += (const ProjectionData& other)
{
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("ProjectionData requires same dimensions for '+' operation:\n" +
                                dimensions().info() + " += " + other.dimensions().info());

    auto totalViews = nbViews();
    std::vector<std::future<SingleViewData&>> futures;
    futures.reserve(totalViews);

    for(uint view = 0; view < totalViews; ++view)
        futures.push_back(std::async(&SingleViewData::operator+=,
                                     &_data[view],
                                     std::ref(other._data[view])));
    return *this;
}

/*!
 * Subtracts the data of \a other from this instance and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this instance do not match.
 */
ProjectionData& ProjectionData::operator -= (const ProjectionData& other)
{
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("ProjectionData requires same dimensions for '-' operation:\n" +
                                dimensions().info() + " -= " + other.dimensions().info());

    auto totalViews = nbViews();
    std::vector<std::future<SingleViewData&>> futures;
    futures.reserve(totalViews);

    for(uint view = 0; view < totalViews; ++view)
        futures.push_back(std::async(&SingleViewData::operator-=,
                                     &_data[view],
                                     std::ref(other._data[view])));
    return *this;
}

/*!
 * Multiplies all projection data in this instance by \a factor and returns a reference to this
 * instance.
 */
ProjectionData& ProjectionData::operator *= (float factor)
{
    for(auto& singleView : _data)
        singleView *= factor;

    return *this;
}

/*!
 * Divides all projection data in this instance by \a divisor and returns a reference to this
 * instance.
 */
ProjectionData& ProjectionData::operator /= (float divisor)
{
    for(auto& singleView : _data)
        singleView /= divisor;

    return *this;
}

/*!
 * Returns the (element-wise) sum of \a other and this instance.
 * Throws an std::domain_error if the dimensions of \a other and this view do not match.
 */
ProjectionData ProjectionData::operator + (const ProjectionData& other) const
{
    ProjectionData ret(*this);
    ret += other;

    return ret;
}

/*!
 * Returns the (element-wise) difference of \a other and this instance.
 * Throws an std::domain_error if the dimensions of \a other and this view do not match.
 */
ProjectionData ProjectionData::operator - (const ProjectionData& other) const
{
    ProjectionData ret(*this);
    ret -= other;

    return ret;
}

/*!
 * Multiplies all projection data in this instance by \a factor and returns the result.
 */
ProjectionData ProjectionData::operator * (float factor) const
{
    ProjectionData ret(*this);
    ret *= factor;

    return ret;
}

/*!
 * Divides all projection data in this instance by \a divisor and returns the result.
 */
ProjectionData ProjectionData::operator / (float divisor) const
{
    ProjectionData ret(*this);
    ret /= divisor;

    return ret;
}

/*!
 * Returns true if all three dimensions of \a other are equal to those of this instance.
 */
bool ProjectionData::Dimensions::operator==(const Dimensions &other) const
{
    return (nbChannels == other.nbChannels) && (nbRows == other.nbRows)
            && (nbModules == other.nbModules) && (nbViews == other.nbViews);
}

/*!
 * Returns true if at least one dimensions of \a other is different from the value in this instance.
 */
bool ProjectionData::Dimensions::operator!=(const Dimensions &other) const
{
    return (nbChannels != other.nbChannels) || (nbRows != other.nbRows)
            || (nbModules != other.nbModules) || (nbViews != other.nbViews);
}

/*!
 * Returns a string containing the dimension values, joined by " x ".
 */
std::string ProjectionData::Dimensions::info() const
{
    return std::to_string(nbChannels) + " x " + std::to_string(nbRows) + " x "
            + std::to_string(nbModules) + " x " + std::to_string(nbViews);
}

/*!
 * Returns the total number of elements for data with this dimensions.
 */
uint ProjectionData::Dimensions::totalNbElements() const
{
    return nbChannels * nbRows * nbModules * nbViews;
}

/*!
 * Returns a constant reference to the stored data vector.
 */
const std::vector<SingleViewData>& ProjectionData::constData() const { return _data; }

/*!
 * Returns a constant reference to the stored data vector.
 */
const std::vector<SingleViewData>& ProjectionData::data() const { return _data; }

/*!
 * Returns a reference to the stored data vector.
 */
std::vector<SingleViewData>& ProjectionData::data() { return _data; }

/*!
 * Returns the dimensions of the data. This contains the number of views (\c nbViews), the number of
 * modules in each view (\c nbModules) and the dimensions of individual modules, namely module width
 * (\c nbChannels) and module height (\c nbRows).
 */
ProjectionData::Dimensions ProjectionData::dimensions() const
{
    return { _viewDim.nbChannels, _viewDim.nbRows, _viewDim.nbModules, nbViews() };
}

/*!
 * Returns the number of views in the data.
 */
uint ProjectionData::nbViews() const { return static_cast<uint>(_data.size()); }

/*!
 * Returns a (modifiable) reference to the SingleViewData of view \a i.
 */
SingleViewData& ProjectionData::view(uint i)
{
    Q_ASSERT(i < nbViews());
    return _data.at(i);
}

/*!
 * Returns a constant reference to the SingleViewData of view \a i.
 */
const SingleViewData& ProjectionData::view(uint i) const
{
    Q_ASSERT(i < nbViews());
    return _data.at(i);
}

/*!
 * Returns the dimensions of the individual single views in the dataset. This contains the number of
 * modules (\c nbModules) and the dimensions of individual modules (\c nbChannels and \c nbRows).
 */
SingleViewData::Dimensions ProjectionData::viewDimensions() const
{
    return _viewDim;
}

} // namespace CTL
