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
        uint nbChannels; //!< Number of channels in each module.
        uint nbRows;     //!< Number of rows in each module.
        uint nbModules;  //!< Number of modules.
        uint nbViews;    //!< Number of views.

        bool operator==(const Dimensions& other) const;
        bool operator!=(const Dimensions& other) const;
        std::string info() const;
        uint totalNbElements() const;
    };

    explicit ProjectionData(const SingleViewData::Dimensions& viewDimensions);
    ProjectionData(uint channelsPerModule, uint rowsPerModule, uint nbModules);
    ProjectionData(const SingleViewData& singleViewData);
    ProjectionData(SingleViewData&& singleViewData);

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
    void allocateMemory(uint nbViews, float initValue);
    void append(SingleViewData&& singleView);
    void append(const SingleViewData& singleView);
    ProjectionData combined(const ModuleLayout& layout = ModuleLayout()) const;
    void fill(float fillValue);
    float max() const;
    float min() const;
    void setDataFromVector(const std::vector<float>& dataVector);
    std::vector<float> toVector() const;
    void transformToExtinction(double i0 = 1.0);
    void transformToExtinction(const std::vector<double>& viewDependentI0);
    void transformToIntensity(double i0 = 1.0);
    void transformToIntensity(const std::vector<double>& viewDependentI0);
    void transformToCounts(double n0 = 1.0);
    void transformToCounts(const std::vector<double>& viewDependentN0);

    bool operator==(const ProjectionData& other) const;
    bool operator!=(const ProjectionData& other) const;

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

} // namespace CTL

/*! \file */

#endif // PROJECTIONDATA_H
