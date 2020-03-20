#ifndef CTL_TRIVIALDYNAMICVOLUME_H
#define CTL_TRIVIALDYNAMICVOLUME_H

#include "abstractdynamicvolumedata.h"
#include <cmath>

/*
 * NOTE: This is header only.
 */

namespace CTL {

class TrivialDynamicVolume : public AbstractDynamicVolumeData
{
    // abstract interface
    protected: void updateVolume() override;
    public: SpectralVolumeData* clone() const override;

public:
    using AbstractDynamicVolumeData::AbstractDynamicVolumeData;
};

inline SpectralVolumeData* TrivialDynamicVolume::clone() const
{
    return new TrivialDynamicVolume(*this);
}

/*!
 * Scaling with a factor > 1 that linearly increases with the time (example implementation).
 */
inline void TrivialDynamicVolume::updateVolume() { *this *= float(std::abs(0.01 * time()) + 1.0); }

} // namespace CTL

#endif // CTL_TRIVIALDYNAMICVOLUME_H
