#ifndef ABSTRACTDYNAMICVOLUMEDATA_H
#define ABSTRACTDYNAMICVOLUMEDATA_H

#include "spectralvolumedata.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {

class AbstractDynamicVolumeData : public SpectralVolumeData
{
    // abstract interface
protected:virtual void updateVolume() = 0;
public:virtual SpectralVolumeData* clone() const override = 0;

public:
    AbstractDynamicVolumeData(const SpectralVolumeData& other);

    void setTime(float seconds);
    float time() const;

protected:
    AbstractDynamicVolumeData(const AbstractDynamicVolumeData&) = default;
    AbstractDynamicVolumeData(AbstractDynamicVolumeData&&) = default;
    AbstractDynamicVolumeData& operator=(const AbstractDynamicVolumeData&) = default;
    AbstractDynamicVolumeData& operator=(AbstractDynamicVolumeData&&) = default;

private:
    float _time = 0.0f; //!< current time in milliseconds
};

/*!
 * Initializes the dynamic volume using a static VoxelVolume
 */
inline AbstractDynamicVolumeData::AbstractDynamicVolumeData(const SpectralVolumeData& other)
    : SpectralVolumeData(other)
{
}

inline void AbstractDynamicVolumeData::setTime(float seconds)
{
    _time = seconds;
    updateVolume();
}

inline float AbstractDynamicVolumeData::time() const { return _time; }


} // namespace CTL

#endif // ABSTRACTDYNAMICVOLUMEDATA_H
