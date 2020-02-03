#include "compositevolume.h"

namespace CTL {

// ### CompositeVolume ###

const SpectralVolumeData& CompositeVolume::materialVolume(uint materialIdx) const
{
    return _materialVolumes[materialIdx];
}

SpectralVolumeData CompositeVolume::muVolume(uint materialIdx, float centerEnergy, float binWidth) const
{
    return _materialVolumes[materialIdx].muVolume(centerEnergy, binWidth);
}

uint CompositeVolume::nbMaterials() const
{
    return static_cast<uint>(_materialVolumes.size());
}

void CompositeVolume::addMaterialVolume(const SpectralVolumeData& volume)
{
    _materialVolumes.push_back(volume);
}

void CompositeVolume::addMaterialVolume(SpectralVolumeData&& volume)
{
    _materialVolumes.push_back(std::move(volume));
}

}
