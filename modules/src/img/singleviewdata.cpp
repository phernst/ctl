#include "singleviewdata.h"
#include <cmath>
#include <QDebug>

namespace CTL
{

/*!
 * Constructs a SingleViewData object with dimensions for the individual module data as specified
 * by \a moduleDimensions. This does not allocate any memory for the actual data. To (explicitely)
 * do so, use allocateMemory().
 */
SingleViewData::SingleViewData(const ModuleData::Dimensions &moduleDimensions)
    : _moduleDim(moduleDimensions)
{

}

/*!
 * Constructs a SingleViewData object with individual module data dimensions of \a channelsPerModule
 * x \a rowsPerModule. This does not allocate any memory for the actual data. To (explicitely)
 * do so, use allocateMemory().
 */
SingleViewData::SingleViewData(uint channelsPerModule, uint rowsPerModule)
    : _moduleDim({channelsPerModule, rowsPerModule})
{

}

/*!
 * Appends the data from \a moduleData to this single view. The dimensions of \a moduleData must
 * match the dimensions of this view. Throws std::domain_error in case of mismatching dimensions.
 *
 * Overloaded method that uses move-semantics. \sa append(const ModuleData &)
 */
void SingleViewData::append(ModuleData &&moduleData)
{
    if(!hasEqualSizeAs(moduleData))
        throw std::domain_error("ModuleData has incompatible size for SingleViewData:\n[" +
                                _moduleDim.info() + "].append([" + moduleData.dimensions().info() + "])");

     _data.push_back(std::move(moduleData));
}

/*!
 * Appends the data from \a moduleData to this single view. The dimensions of \a moduleData must
 * match the dimensions of this view. Throws std::domain_error in case of mismatching dimensions.
 *
 * \sa append(ModuleData&&)
 */
void SingleViewData::append(const ModuleData &moduleData)
{
    if(!hasEqualSizeAs(moduleData))
        throw std::domain_error("ModuleData has incompatible size for SingleViewData:\n[" +
                                _moduleDim.info() + "].append([" + moduleData.dimensions().info() + "])");

     _data.push_back(moduleData);
}

/*!
 * Constructs a Chunk2D from the data in \a dataVector and appends it to this single view.
 * \a dataVector must have the same number of elements that are required for the module data in
 * this view, i.e. \c nbChannels x \c nbRows.Throws std::domain_error in case of mismatching dimensions.
 *
 * Overloaded method that uses move-semantics. \sa append(const std::vector<float>&)
 */
void SingleViewData::append(std::vector<float> &&dataVector)
{
    if(!hasEqualSizeAs(dataVector))
        throw std::domain_error("data vector has incompatible size for appending to SingleViewData");

    _data.emplace_back(_moduleDim.width, _moduleDim.height, std::move(dataVector));
}

/*!
 * Constructs a Chunk2D from the data in \a dataVector and appends it to this single view.
 * \a dataVector must have the same number of elements that are required for the module data in
 * this view, i.e. \c nbChannels x \c nbRows.Throws std::domain_error in case of mismatching dimensions.
 *
 * \sa append(std::vector<float>&&)
 */
void SingleViewData::append(const std::vector<float> &dataVector)
{
    if(!hasEqualSizeAs(dataVector))
        throw std::domain_error("data vector has incompatible size for appending to SingleViewData");

    _data.emplace_back(_moduleDim.width, _moduleDim.height, dataVector);
}

/*!
 * Concatenates the projection data from all modules into a vector and returns the result.
 *
 * Data is concatenated in row major order, i.e. the vector starts with the data from the first row
 * of the first module, followed by the remaining rows of that module. Subsequently, the next modules
 * are appended with the same concept.
 */
std::vector<float> SingleViewData::toVector() const
{
    std::vector<float> ret(totalPixelCount());

    for(uint mod=0; mod<nbModules(); ++mod)
        std::copy_n(module(mod).rawData(), elementsPerModule(), ret.data() + mod*elementsPerModule());

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
void SingleViewData::transformToExtinction(double i0)
{
    for(auto& chunk : _data)
        for(auto& pix : chunk.data())
            pix = log(i0/pix);
}

/*!
 * Transforms all data values in this instance to intensities (w.r.t. the initial intensity passed
 * by \a i0) using the following formula:
 *
 * \f$
 * \mathtt{newValue}=i0\cdot\exp(-\mathtt{oldValue}).
 * \f$
 */
void SingleViewData::transformToIntensity(double i0)
{
    for(auto& chunk : _data)
        for(auto& pix : chunk.data())
            pix = i0 * exp(-pix);
}

/*!
 * Sets the data of this instance based on the data given by \a dataVector. For this, it
 * is assumed that the data in \a dataVector has row major order, i.e. all values of a module row,
 * followed by the remaining rows and the other modules.
 *
 * This method is provided for convenience to serve as an alternative to use append() for individual
 * modules.
 */
void SingleViewData::setDataFromVector(const std::vector<float> &dataVector)
{
    const size_t elementsPerModule = _moduleDim.height*_moduleDim.width;

    Q_ASSERT(elementsPerModule);
    if(elementsPerModule == 0)
        throw std::domain_error("SingleViewData has null-dimension");

    // check if number of elements in 'dataVector' is multiple of required number of elements in a single view (specified in '_viewDim')
    Q_ASSERT(dataVector.size() % elementsPerModule == 0);
    if(dataVector.size() % elementsPerModule != 0)
        throw std::domain_error("data vector has incompatible size for ProjectionData");

    // start building new data
    _data.clear(); // clear all previous data

    auto nbModulesToBuild = dataVector.size() / elementsPerModule;

    for(uint module=0; module < nbModulesToBuild; ++module)
    {
        std::vector<float> tmp(elementsPerModule);
        std::copy_n(dataVector.data() + module*elementsPerModule, elementsPerModule, tmp.data());
        append(std::move(tmp));
    }
}

/*!
 * Combines the projection data from all modules into a single Chunk2D and returns the result.
 *
 * To combine the data, a \a layout is required that describes the arrangement of the individual modules.
 */
Chunk2D<float> SingleViewData::combined(const ModuleLayout &layout, bool *ok) const
{
    if(ok) *ok = true;

    if(layout.isEmpty())
        return combined(ModuleLayout::canonicLayout(1, std::max(nbModules(), 1u)), ok);

    const uint nbRows = layout.rows();
    const uint nbCols = layout.columns();
    const uint elemPerMod = elementsPerModule();

    Chunk2D<float> combinedChunk(nbCols*_moduleDim.width, nbRows*_moduleDim.height); // the result chunk

    // prepare data for combined chunk
    std::vector<float> temp(combinedChunk.nbElements(), 0.0f);
    for(uint row=0; row<nbRows; ++row)
        for(uint col=0; col<nbCols; ++col)
        {
            if(layout(row,col) < 0) // skip layout tile if module ID is negative
            {
                qDebug() << "Module position (" << row << "," << col << ") skipped. [index:" << layout(row,col) << "]";
                continue;
            }
            auto moduleID = static_cast<uint>(layout(row,col)); // data from this module goes at position (row,col) in the combined chunk

            Q_ASSERT(moduleID < nbModules()); // check if module is available in the data
            if(moduleID >= nbModules())
            {
                if(ok) *ok = false;
                continue;
            }

            uint skip = (row*nbCols*elemPerMod) + (col*_moduleDim.width); // skip all data from earlier modules (i.e. full rows + columns within current row)
            float* resPtr   = temp.data() + skip;
            const float* currentModulePtr = module(moduleID).rawData();

            for(uint moduleRow=0; moduleRow<_moduleDim.height; ++moduleRow)
            {
                std::copy_n(currentModulePtr, _moduleDim.width, resPtr);
                currentModulePtr += _moduleDim.width; // set (source) pointer to beginning of next row in the current module
                resPtr += combinedChunk.width(); // shift (result) pointer by one full row in the combined chunk
            }
        }

    combinedChunk.setData(std::move(temp));

    return combinedChunk;
}

/*!
 * Returns the maximum value in this instance.
 *
 * Returns zero if this data is empty.
 */
float SingleViewData::max() const
{
    if(nbModules() == 0)
        return 0.0f;

    float tmpMax = module(0).max();

    float locMax;
    for(int mod = 1, total = nbModules(); mod < total; ++mod)
    {
        locMax = module(mod).max();
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
float SingleViewData::min() const
{
    if(nbModules() == 0)
        return 0.0f;

    float tmpMin = module(0).min();

    float locMin;
    for(int mod = 1, total = nbModules(); mod < total; ++mod)
    {
        locMin = module(mod).min();
        if(locMin < tmpMin)
            tmpMin = locMin;
    }

    return tmpMin;
}

/*!
 * Returns true if the dimensions of \a other are equal to those of this instance.
 */
bool SingleViewData::hasEqualSizeAs(const ModuleData &other) const
{
    Q_ASSERT(other.dimensions() == _moduleDim);
    return other.dimensions() == _moduleDim;
}

/*!
 * Returns true if the number of elements in \a other is equal to the number of elements
 * in an individual module of this instance.
 */
bool SingleViewData::hasEqualSizeAs(const std::vector<float> &other) const
{
    Q_ASSERT(other.size() == elementsPerModule());
    return other.size() == elementsPerModule();
}

/*!
 * Enforces memory allocation. This resizes the internal std::vector to the required number of
 * modules and requests memory allocation for each of the modules.
 * As a result, the number of modules is equal to \a nbModules.
 */
void SingleViewData::allocateMemory(uint nbModules)
{
    _data.resize(nbModules, ModuleData(_moduleDim));

    for(auto& module : _data)
        module.allocateMemory();
}

/*!
 * Adds the data from \a other to this view and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this view do not match.
 */
SingleViewData& SingleViewData::operator += (const SingleViewData& other)
{
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("SingleViewData requires same dimensions for '+' operation:\n" +
                                dimensions().info() + " += " + other.dimensions().info());

    auto otherIt = other.constData().cbegin();
    for(auto& chunk : _data)
        chunk += *otherIt++;

    return *this;
}

/*!
 * Subtracts the data of \a other from this view and returns a reference to this instance.
 * Throws an std::domain_error if the dimensions of \a other and this view do not match.
 */
SingleViewData& SingleViewData::operator -= (const SingleViewData& other)
{
    Q_ASSERT(dimensions() == other.dimensions());
    if(dimensions() != other.dimensions())
        throw std::domain_error("SingleViewData requires same dimensions for '-' operation:\n" +
                                dimensions().info() + " -= " + other.dimensions().info());

    auto otherIt = other.constData().cbegin();
    for(auto& chunk : _data)
        chunk -= *otherIt++;

    return *this;
}

/*!
 * Multiplies all projection data in this view by \a factor and returns a reference to this instance.
 */
SingleViewData& SingleViewData::operator *= (float factor)
{
    for(auto& chunk : _data)
        chunk *= factor;

    return *this;
}

/*!
 * Divides all projection data in this view by \a divisor and returns a reference to this instance.
 */
SingleViewData& SingleViewData::operator /= (float divisor)
{
    for(auto& chunk : _data)
        chunk /= divisor;

    return *this;
}

/*!
 * Adds the data from \a other to this view and returns the result.
 * Throws an std::domain_error if the dimensions of \a other and this view do not match.
 */
SingleViewData SingleViewData::operator + (const SingleViewData& other) const
{
    SingleViewData ret(*this);
    ret += other;

    return ret;
}

/*!
 * Subtracts the data of \a other from this view and returns the result.
 * Throws an std::domain_error if the dimensions of \a other and this view do not match.
 */
SingleViewData SingleViewData::operator - (const SingleViewData& other) const
{
    SingleViewData ret(*this);
    ret -= other;

    return ret;
}

/*!
 * Multiplies all projection data in this view by \a factor and returns the result.
 */
SingleViewData SingleViewData::operator * (float factor) const
{
    SingleViewData ret(*this);
    ret *= factor;

    return ret;
}

/*!
 * Divides all projection data in this view by \a divisor and returns the result.
 */
SingleViewData SingleViewData::operator / (float divisor) const
{
    SingleViewData ret(*this);
    ret /= divisor;

    return ret;
}

/*!
 * Returns true if all three dimensions of \a other are equal to those of this instance.
 */
bool SingleViewData::Dimensions::operator==(const Dimensions &other) const
{
    return (nbChannels == other.nbChannels) && (nbRows == other.nbRows)
            && (nbModules == other.nbModules);
}

/*!
 * Returns true if at least one dimensions of \a other is different from the value in this instance.
 */
bool SingleViewData::Dimensions::operator!=(const Dimensions &other) const
{
    return (nbChannels != other.nbChannels) || (nbRows != other.nbRows)
            || (nbModules != other.nbModules);
}

/*!
 * Returns a string containing the dimension values, joined by " x ".
 */
std::string SingleViewData::Dimensions::info() const
{
    return std::to_string(nbChannels) + " x " + std::to_string(nbRows) + " x "
            + std::to_string(nbModules);
}

/*!
 * Returns a constant reference to the stored data vector.
 */
const std::vector<SingleViewData::ModuleData>& SingleViewData::constData() const
{
    return _data;
}

/*!
 * Returns a constant reference to the stored data vector.
 */
const std::vector<SingleViewData::ModuleData>& SingleViewData::data() const { return _data; }

/*!
 * Returns a reference to the stored data vector.
 */
std::vector<SingleViewData::ModuleData>& SingleViewData::data() { return _data; }

/*!
 * Returns the dimensions of the data. This contains module width (\c nbChannels), module height
 * (\c nbRows) and the number of modules (\c nbModules).
 */
SingleViewData::Dimensions SingleViewData::dimensions() const
{
    return { _moduleDim.width, _moduleDim.height, nbModules() };
}

/*!
 * Returns the number of elements (or pixels) per module.
 *
 * Same as: dimensions().nbChannels * dimensions().nbRows.
 */
uint SingleViewData::elementsPerModule() const
{
    return _moduleDim.width * _moduleDim.height;
}

/*!
 * Returns a (modifiable) reference to the projection data of module \a i. Does not perform boundary
 * checks.
 */
SingleViewData::ModuleData& SingleViewData::module(uint i)
{
    Q_ASSERT(i < nbModules());
    return _data.at(i);
}

/*!
 * Returns a constant reference to the projection data of module \a i. Does not perform boundary
 * checks.
 */
const SingleViewData::ModuleData& SingleViewData::module(uint i) const
{
    Q_ASSERT(i < nbModules());
    return _data.at(i);
}

/*!
 * Returns the number of modules.
 */
uint SingleViewData::nbModules() const { return static_cast<uint>(_data.size()); }

/*!
 * Returns the total number of pixels in the data. This computes as
 * \c nbChannels x \c nbRows x \c nbModules.
 *
 * Same as: nbModules() * elementsPerModule().
 */
size_t SingleViewData::totalPixelCount() const
{
    return static_cast<size_t>(nbModules()) * elementsPerModule();
}

} // namespace CTL
