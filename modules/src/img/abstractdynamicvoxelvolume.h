#ifndef ABSTRACTDYNAMICVOXELVOLUME_H
#define ABSTRACTDYNAMICVOXELVOLUME_H

#include "voxelvolume.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {

class AbstractDynamicVoxelVolume : public VoxelVolume<float>
{
    // abstract interface
    protected:virtual void updateVolume() = 0;

public:
    AbstractDynamicVoxelVolume(const VoxelVolume<float>& other);

    virtual ~AbstractDynamicVoxelVolume() = default;

    void setTime(float seconds);
    float time() const;

private:
    float _time = 0.0f; //!< current time in seconds
};

/*!
 * Initializes the dynamic volume using a static VoxelVolume
 */
inline AbstractDynamicVoxelVolume::AbstractDynamicVoxelVolume(const VoxelVolume<float>& other)
    : VoxelVolume<float>(other)
{
}

inline void AbstractDynamicVoxelVolume::setTime(float seconds)
{
    _time = seconds;
    updateVolume();
}

inline float AbstractDynamicVoxelVolume::time() const { return _time; }

} // namespace CTL

#endif // ABSTRACTDYNAMICVOXELVOLUME_H
