#ifndef PROJECTIONDATA_H
#define PROJECTIONDATA_H

#include <iostream>

#include "singleviewdata.h"

namespace CTL {

/*!
 * \class ProjectionData
 *
 * \brief The ProjectionData class is the container class used to store all projections from
 * all views.
 *
 * This container holds all projection data in a vector of SingleViewData objects. Individual single
 * views can be added using append(). Alternatively, the entire data can be set from a std::vector
 * using setDataFromVector(). This might be useful to convert data from other sources that provide
 * the projection as a one dimensional memory block.
 */
class ProjectionData
{
public:
    struct Dimensions
    {
        uint nbChannels;
        uint nbRows;
        uint nbModules;
        uint nbViews;

        bool operator==(const Dimensions& other) const;
        bool operator!=(const Dimensions& other) const;
        std::string info() const;
        uint totalNbElements() const;
    };

    ProjectionData(const SingleViewData::Dimensions& viewDimensions);
    ProjectionData(uint channelsPerModule, uint rowsPerModule, uint nbModules);

    // getter methods
    const std::vector<SingleViewData>& constData() const;
    const std::vector<SingleViewData>& data() const;
    std::vector<SingleViewData>& data();
    Dimensions dimensions() const;
    uint nbViews() const;
    SingleViewData& view(uint i);
    const SingleViewData& view(uint i) const;
    SingleViewData::Dimensions viewDimensions() const;

    // other methods
    void allocateMemory(uint nbViews);
    void append(SingleViewData&& singleView);
    void append(const SingleViewData& singleView);
    ProjectionData combined(const ModuleLayout& layout) const;
    float max() const;
    float min() const;
    void setDataFromVector(const std::vector<float>& dataVector);
    std::vector<float> toVector() const;
    void transformToExtinction(double i0 = 1.0);
    void transformToIntensity(double i0 = 1.0);

    // arithmetic operations
    ProjectionData& operator+=(const ProjectionData& other);
    ProjectionData& operator-=(const ProjectionData& other);
    ProjectionData& operator*=(float factor);
    ProjectionData& operator/=(float divisor);

    ProjectionData operator+(const ProjectionData& other) const;
    ProjectionData operator-(const ProjectionData& other) const;
    ProjectionData operator*(float factor) const;
    ProjectionData operator/(float divisor) const;

protected:
    SingleViewData::Dimensions _viewDim; //!< The dimensions of the individual single views.

    std::vector<SingleViewData> _data; //!< The internal data storage vector.

private:
    bool hasEqualSizeAs(const SingleViewData& other) const;
};

/*!
 * \struct ProjectionData::Dimensions
 * \brief Struct that holds the dimensions of a ProjectionData object.
 *
 * This contains the number of views (\c nbViews), the number of modules in each view (\c nbModules)
 * and the dimensions of individual modules, namely module width (\c nbChannels) and module height.
 */

/*!
 * Returns true if all three dimensions of \a other are equal to those of this instance.
 */
inline bool ProjectionData::Dimensions::operator==(const Dimensions &other) const
{
    return (nbChannels == other.nbChannels) && (nbRows == other.nbRows)
            && (nbModules == other.nbModules) && (nbViews == other.nbViews);
}

/*!
 * Returns true if at least one dimensions of \a other is different from the value in this instance.
 */
inline bool ProjectionData::Dimensions::operator!=(const Dimensions &other) const
{
    return (nbChannels != other.nbChannels) || (nbRows != other.nbRows)
            || (nbModules != other.nbModules) || (nbViews != other.nbViews);
}

/*!
 * Returns a string containing the dimension values, joined by " x ".
 */
inline std::string ProjectionData::Dimensions::info() const
{
    return std::to_string(nbChannels) + " x " + std::to_string(nbRows) + " x "
            + std::to_string(nbModules) + " x " + std::to_string(nbViews);
}

/*!
 * Returns the total number of elements for data with this dimensions.
 */
inline uint ProjectionData::Dimensions::totalNbElements() const
{
    return nbChannels * nbRows * nbModules * nbViews;
}

/*!
 * Returns a constant reference to the stored data vector.
 */
inline const std::vector<SingleViewData>& ProjectionData::constData() const { return _data; }

/*!
 * Returns a constant reference to the stored data vector.
 */
inline const std::vector<SingleViewData>& ProjectionData::data() const { return _data; }

/*!
 * Returns a reference to the stored data vector.
 */
inline std::vector<SingleViewData>& ProjectionData::data() { return _data; }

/*!
 * Returns the dimensions of the data. This contains the number of views (\c nbViews), the number of
 * modules in each view (\c nbModules) and the dimensions of individual modules, namely module width
 * (\c nbChannels) and module height (\c nbRows).
 */
inline ProjectionData::Dimensions ProjectionData::dimensions() const
{
    return { _viewDim.nbChannels, _viewDim.nbRows, _viewDim.nbModules, nbViews() };
}

/*!
 * Returns the number of views in the data.
 */
inline uint ProjectionData::nbViews() const { return static_cast<uint>(_data.size()); }

/*!
 * Returns a (modifiable) reference to the SingleViewData of view \a i.
 */
inline SingleViewData& ProjectionData::view(uint i)
{
    Q_ASSERT(i < nbViews());
    return _data.at(i);
}

/*!
 * Returns a constant reference to the SingleViewData of view \a i.
 */
inline const SingleViewData& ProjectionData::view(uint i) const
{
    Q_ASSERT(i < nbViews());
    return _data.at(i);
}

/*!
 * Returns the dimensions of the individual single views in the dataset. This contains the number of
 * modules (\c nbModules) and the dimensions of individual modules (\c nbChannels and \c nbRows).
 */
inline SingleViewData::Dimensions ProjectionData::viewDimensions() const
{
    return _viewDim;
}

} // namespace CTL

/*! \file */

#endif // PROJECTIONDATA_H
