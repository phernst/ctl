#include "compositevolume.h"

namespace CTL {

// ### CompositeVolume ###

const SpectralVolumeData& CompositeVolume::subVolume(uint materialIdx) const
{
    return _subVolumes[materialIdx];
}

SpectralVolumeData CompositeVolume::muVolume(uint materialIdx, float centerEnergy, float binWidth) const
{
    return _subVolumes[materialIdx].muVolume(centerEnergy, binWidth);
}

uint CompositeVolume::nbSubVolumes() const
{
    return static_cast<uint>(_subVolumes.size());
}

void CompositeVolume::addSubVolume(SpectralVolumeData volume)
{
    _subVolumes.push_back(std::move(volume));
}

}
