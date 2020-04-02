#ifndef CTL_ABSTRACTDYNAMICVOLUMEDATA_H
#define CTL_ABSTRACTDYNAMICVOLUMEDATA_H

#include "spectralvolumedata.h"
#include "models/xydataseries.h"
#include "processing/coordinates.h" // for Range<T>

/*
 * NOTE: This is header only.
 */

namespace CTL {

class AbstractDynamicVolumeData : public SpectralVolumeData
{
    // abstract interface
    protected:virtual void updateVolume() = 0;
    public: SpectralVolumeData* clone() const override = 0;

public:
    AbstractDynamicVolumeData(const SpectralVolumeData& other);

    virtual XYDataSeries timeCurve(uint x, uint y, uint z, const std::vector<float> &timePoints);

    void setTime(double seconds);
    double time() const;
    XYDataSeries timeCurve(uint x, uint y, uint z, float tStart, float tEnd, uint nbSamples);
    XYDataSeries timeCurve(uint x, uint y, uint z, SamplingRange timeRange, uint nbSamples);

protected:
    AbstractDynamicVolumeData(const AbstractDynamicVolumeData&) = default;
    AbstractDynamicVolumeData(AbstractDynamicVolumeData&&) = default;
    AbstractDynamicVolumeData& operator=(const AbstractDynamicVolumeData&) = default;
    AbstractDynamicVolumeData& operator=(AbstractDynamicVolumeData&&) = default;

private:
    double _time = 0.0; //!< current time in milliseconds
};

/*!
 * Initializes the dynamic volume using a static VoxelVolume
 */
inline AbstractDynamicVolumeData::AbstractDynamicVolumeData(const SpectralVolumeData& other)
    : SpectralVolumeData(other)
{
}

/*!
 * Returns the data series containing the data of voxel (\a x, \a y, \a z) at the time points
 * specified in \a timePoints (to be specified in ms).
 *
 * Note that this is a highly inefficient implementation that requires updating the entire volume
 * for each requested time point using updateVolume(). Make sure to override this method in sub-
 * classes to use more efficient ways of evaluating values of the requested voxel alone, if
 * possible.
 */
inline XYDataSeries AbstractDynamicVolumeData::timeCurve(uint x, uint y, uint z,
                                                         const std::vector<float>& timePoints)
{
    XYDataSeries ret;
    const auto timePtCache = time();

    for(const auto& smp : timePoints)
    {
        setTime(smp);
        ret.append(smp, this->operator()(x,y,z));
    }

    // reset time point to initial value
    setTime(timePtCache);

    return ret;
}

inline void AbstractDynamicVolumeData::setTime(double seconds)
{
    _time = seconds;
    updateVolume();
}

inline double AbstractDynamicVolumeData::time() const { return _time; }

/*!
 * Convenience alternative of timeCurve(uint, uint, uint, const std::vector<float>&).
 *
 * Returns the time series sampled at \a nbSamples equally-spaced positions within the interval
 * [\a tStart, \a tEnd] (in ms).
 */
inline XYDataSeries AbstractDynamicVolumeData::timeCurve(uint x, uint y, uint z,
                                                         float tStart, float tEnd, uint nbSamples)
{
    return timeCurve(x, y, z, SamplingRange(tStart, tEnd).linspace(nbSamples));
}

/*!
 * Convenience alternative of timeCurve(uint, uint, uint, const std::vector<float>&).
 *
 * Returns the time series sampled at \a nbSamples equally-spaced positions within the interval
 * specified by \a timeRange.
 *
 * Same as: \code timeCurve(x, y, z, timeRange.linspace(nbSamples)); \endcode
 */
inline XYDataSeries AbstractDynamicVolumeData::timeCurve(uint x, uint y, uint z,
                                                         SamplingRange timeRange, uint nbSamples)
{
    return timeCurve(x, y, z, timeRange.linspace(nbSamples));
}

} // namespace CTL

#endif // CTL_ABSTRACTDYNAMICVOLUMEDATA_H
