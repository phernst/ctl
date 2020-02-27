#ifndef TRIVIALDYNAMICVOLUME_H
#define TRIVIALDYNAMICVOLUME_H

#include "abstractdynamicvoxelvolume.h"
#include <cmath>
/*
 * NOTE: This is header only.
 */

namespace CTL {

class TrivialDynamicVolume : public AbstractDynamicVolumeData
{
public: SpectralVolumeData* clone() const override;

public:
    using AbstractDynamicVolumeData::AbstractDynamicVolumeData;

protected:
    void updateVolume() override;
};

SpectralVolumeData* TrivialDynamicVolume::clone() const
{
    return new TrivialDynamicVolume(*this);
}

/*!
 * Does nothing, i.e. values of the volume are not changed.
 */
inline void TrivialDynamicVolume::updateVolume() { *this *= (std::fabs(std::sin(time()))/10.0f + 1.1f); }

} // namespace CTL

#endif // TRIVIALDYNAMICVOLUME_H
