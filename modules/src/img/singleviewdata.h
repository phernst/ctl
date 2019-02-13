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
    Chunk2D<float> combined(const ModuleLayout& layout = ModuleLayout(), bool* ok = nullptr) const;
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

} // namespace CTL

/*! \file */

#endif // SINGLEVIEWDATA_H
