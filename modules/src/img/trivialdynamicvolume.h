#ifndef TRIVIALDYNAMICVOLUME_H
#define TRIVIALDYNAMICVOLUME_H

#include "abstractdynamicvoxelvolume.h"

/*
 * NOTE: This is header only.
 */

namespace CTL {

class TrivialDynamicVolume : public AbstractDynamicVoxelVolume
{
public:
    using AbstractDynamicVoxelVolume::AbstractDynamicVoxelVolume;

protected:
    void updateVolume() override;
};

/*!
 * Does nothing, i.e. values of the volume are not changed.
 */
inline void TrivialDynamicVolume::updateVolume() {}

} // namespace CTL

#endif // TRIVIALDYNAMICVOLUME_H
