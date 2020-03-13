#include "compositevolume.h"

namespace CTL {

// ### CompositeVolume ###

const SpectralVolumeData& CompositeVolume::subVolume(uint materialIdx) const
{
    return *_subVolumes[materialIdx];
}

std::unique_ptr<SpectralVolumeData>
CompositeVolume::muVolume(uint materialIdx, float centerEnergy, float binWidth) const
{
    return _subVolumes[materialIdx]->muVolume(centerEnergy, binWidth);
}

uint CompositeVolume::nbSubVolumes() const { return static_cast<uint>(_subVolumes.size()); }

void CompositeVolume::addSubVolume(SpectralVolumeData volume)
{
    _subVolumes.emplace_back(new SpectralVolumeData(std::move(volume)));
}

void CompositeVolume::addSubVolume(std::unique_ptr<SpectralVolumeData> volume)
{
    _subVolumes.push_back(std::move(volume));
}

void CompositeVolume::addSubVolume(const AbstractDynamicVolumeData& volume)
{
    _subVolumes.emplace_back(volume.clone());
}

void CompositeVolume::addSubVolume(CompositeVolume&& volume)
{
    std::for_each(volume._subVolumes.begin(), volume._subVolumes.end(),
                  [this] (std::unique_ptr<SpectralVolumeData>& vol) { _subVolumes.push_back(std::move(vol)); });
}

void CompositeVolume::addSubVolume(const CompositeVolume& volume)
{
    std::for_each(volume._subVolumes.cbegin(), volume._subVolumes.cend(),
                  [this] (const std::unique_ptr<SpectralVolumeData>& vol) { _subVolumes.emplace_back(vol->clone()); });
}

} // namespace CTL
