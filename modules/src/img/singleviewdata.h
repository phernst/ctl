#ifndef CTL_SINGLEVIEWDATA_H
#define CTL_SINGLEVIEWDATA_H

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
    typedef Chunk2D<float> ModuleData; //!< Alias for template specialization Chunk2D<float>.

    struct Dimensions
    {
        uint nbChannels; //!< Number of channels in each module.
        uint nbRows;     //!< Number of rows in each module.
        uint nbModules;  //!< Number of modules.

        bool operator==(const Dimensions& other) const;
        bool operator!=(const Dimensions& other) const;

        std::string info() const;
        size_t totalNbElements() const;
    };

    explicit SingleViewData(const ModuleData::Dimensions& moduleDimensions);
    SingleViewData(uint channelsPerModule, uint rowsPerModule);
    SingleViewData(const ModuleData& moduleData);
    SingleViewData(ModuleData&& moduleData);

    // getter methods
    const std::vector<ModuleData>& constData() const;
    const std::vector<ModuleData>& data() const;
    std::vector<ModuleData>& data();
    Dimensions dimensions() const;
    uint elementsPerModule() const;
    const ModuleData& first() const;
    ModuleData& first();
    ModuleData& module(uint i);
    const ModuleData& module(uint i) const;
    uint nbModules() const;
    size_t totalPixelCount() const;

    // other methods
    void allocateMemory(uint nbModules);
    void allocateMemory(uint nbModules, float initValue);
    void append(ModuleData&& moduleData);
    void append(const ModuleData& moduleData);
    void append(std::vector<float>&& dataVector);
    void append(const std::vector<float>& dataVector);
    Chunk2D<float> combined(const ModuleLayout& layout = ModuleLayout(), bool* ok = nullptr) const;
    void fill(float fillValue);
    void freeMemory();
    float max() const;
    float min() const;
    void setDataFromVector(const std::vector<float>& dataVector);
    std::vector<float> toVector() const;
    void transformToExtinction(double i0orN0 = 1.0);
    void transformToIntensity(double i0 = 1.0);
    void transformToCounts(double n0 = 1.0);

    bool operator==(const SingleViewData& other) const;
    bool operator!=(const SingleViewData& other) const;

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

#endif // CTL_SINGLEVIEWDATA_H
