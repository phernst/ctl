#ifndef CTL_ABSTRACTDYNAMICVOLUMEDATA_H
#define CTL_ABSTRACTDYNAMICVOLUMEDATA_H

#include "spectralvolumedata.h"

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

    void setTime(double seconds);
    double time() const;

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

inline void AbstractDynamicVolumeData::setTime(double seconds)
{
    _time = seconds;
    updateVolume();
}

inline double AbstractDynamicVolumeData::time() const { return _time; }

} // namespace CTL

#endif // CTL_ABSTRACTDYNAMICVOLUMEDATA_H
