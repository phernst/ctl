#ifndef SINGLEVIEWDATA_H
#define SINGLEVIEWDATA_H

#include "chunk2d.h"
#include "modulelayout.h"

namespace CTL {

/*!
 * \class SingleViewData
 *
 * \brief The SingleViewData class is the container class used to store all projections from
 * one particular view.
 *
 * In the generalized case, the detector consists of several individual flat-panel modules. Each of
 * the modules acquires one projection image. The full set of these images are stored in a
 * SingleViewData object.
 */
class SingleViewData
{
public:
    typedef Chunk2D<float> ModuleData;

    struct Dimensions
    {
        uint nbChannels;
        uint nbRows;
        uint nbModules;

        bool operator==(const Dimensions& other) const;
        bool operator!=(const Dimensions& other) const;
        std::string info() const;
    };

    SingleViewData(const ModuleData::Dimensions& moduleDimensions);
    SingleViewData(uint channelsPerModule, uint rowsPerModule);

    // getter methods
    const std::vector<ModuleData>& constData() const;
    const std::vector<ModuleData>& data() const;
    std::vector<ModuleData>& data();
    Dimensions dimensions() const;
    uint elementsPerModule() const;
    ModuleData& module(uint i);
    const ModuleData& module(uint i) const;
    uint nbModules() const;
    size_t totalPixelCount() const;

    // other methods
    void allocateMemory(uint nbModules);
    void append(ModuleData&& moduleData);
    void append(const ModuleData& moduleData);
    void append(std::vector<float>&& dataVector);
    void append(const std::vector<float>& dataVector);
    Chunk2D<float> combined(const ModuleLayout& layout, bool* ok = nullptr) const;
    float max() const;
    float min() const;
    void setDataFromVector(const std::vector<float>& dataVector);
    std::vector<float> toVector() const;
    void transformToExtinction(double i0 = 1.0);
    void transformToIntensity(double i0 = 1.0);

    SingleViewData& operator+=(const SingleViewData& other);
    SingleViewData& operator-=(const SingleViewData& other);
    SingleViewData& operator*=(float factor);
    SingleViewData& operator/=(float divisor);

    SingleViewData operator+(const SingleViewData& other) const;
    SingleViewData operator-(const SingleViewData& other) const;
    SingleViewData operator*(float factor) const;
    SingleViewData operator/(float divisor) const;

protected:
    ModuleData::Dimensions _moduleDim; //!< The dimensions of the individual modules.

    std::vector<ModuleData> _data;  //!< The internal data storage vector.

private:
    bool hasEqualSizeAs(const ModuleData& other) const;
    bool hasEqualSizeAs(const std::vector<float>& other) const;
};


/*!
 * \struct SingleViewData::Dimensions
 * \brief Struct that holds the dimensions, namely number of channels, rows and modules, of a
 * SingleViewData object.
 */

/*!
 * Returns true if all three dimensions of \a other are equal to those of this instance.
 */
inline bool SingleViewData::Dimensions::operator==(const Dimensions &other) const
{
    return (nbChannels == other.nbChannels) && (nbRows == other.nbRows)
            && (nbModules == other.nbModules);
}

/*!
 * Returns true if at least one dimensions of \a other is different from the value in this instance.
 */
inline bool SingleViewData::Dimensions::operator!=(const Dimensions &other) const
{
    return (nbChannels != other.nbChannels) || (nbRows != other.nbRows)
            || (nbModules != other.nbModules);
}

/*!
 * Returns a string containing the dimension values, joined by " x ".
 */
inline std::string SingleViewData::Dimensions::info() const
{
    return std::to_string(nbChannels) + " x " + std::to_string(nbRows) + " x "
            + std::to_string(nbModules);
}

/*!
 * Returns a constant reference to the stored data vector.
 */
inline const std::vector<SingleViewData::ModuleData>& SingleViewData::constData() const
{
    return _data;
}

/*!
 * Returns a constant reference to the stored data vector.
 */
inline const std::vector<SingleViewData::ModuleData>& SingleViewData::data() const { return _data; }

/*!
 * Returns a reference to the stored data vector.
 */
inline std::vector<SingleViewData::ModuleData>& SingleViewData::data() { return _data; }

/*!
 * Returns the dimensions of the data. This contains module width (\c nbChannels), module height
 * (\c nbRows) and the number of modules (\c nbModules).
 */
inline SingleViewData::Dimensions SingleViewData::dimensions() const
{
    return { _moduleDim.width, _moduleDim.height, nbModules() };
}

/*!
 * Returns the number of elements (or pixels) per module.
 *
 * Same as: dimensions().nbChannels * dimensions().nbRows.
 */
inline uint SingleViewData::elementsPerModule() const
{
    return _moduleDim.width * _moduleDim.height;
}

/*!
 * Returns a (modifiable) reference to the projection data of module \a i. Does not perform boundary
 * checks.
 */
inline SingleViewData::ModuleData& SingleViewData::module(uint i)
{
    Q_ASSERT(i < nbModules());
    return _data.at(i);
}

/*!
 * Returns a constant reference to the projection data of module \a i. Does not perform boundary
 * checks.
 */
inline const SingleViewData::ModuleData& SingleViewData::module(uint i) const
{
    Q_ASSERT(i < nbModules());
    return _data.at(i);
}

/*!
 * Returns the number of modules.
 */
inline uint SingleViewData::nbModules() const { return static_cast<uint>(_data.size()); }

/*!
 * Returns the total number of pixels in the data. This computes as
 * \c nbChannels x \c nbRows x \c nbModules.
 *
 * Same as: nbModules() * elementsPerModule().
 */
inline size_t SingleViewData::totalPixelCount() const
{
    return static_cast<size_t>(nbModules()) * elementsPerModule();
}

} // namespace CTL

/*! \file */

#endif // SINGLEVIEWDATA_H
