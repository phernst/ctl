#ifndef COMPOSITEVOLUME_H
#define COMPOSITEVOLUME_H

#include "abstractdynamicvolumedata.h"
#include <deque>

namespace CTL {

class CompositeVolume
{
public:
    CompositeVolume() = default;
    template <class... Volumes>
    CompositeVolume(SpectralVolumeData volume, Volumes&&... otherVolumes);
    template <class... Volumes>
    CompositeVolume(std::unique_ptr<SpectralVolumeData> volume, Volumes&&... otherVolumes);
    template <class... Volumes>
    CompositeVolume(const AbstractDynamicVolumeData& volume, Volumes&&... otherVolumes);

    // getter methods
    const SpectralVolumeData& subVolume(uint materialIdx) const;
    std::unique_ptr<SpectralVolumeData> muVolume(uint materialIdx, float centerEnergy,
                                                 float binWidth) const;
    uint nbSubVolumes() const;

    // other methods
    void addSubVolume(SpectralVolumeData volume);
    void addSubVolume(std::unique_ptr<SpectralVolumeData> volume);
    void addSubVolume(const AbstractDynamicVolumeData& volume);
    void addSubVolume(CompositeVolume&& volume);

private:
    std::deque<std::unique_ptr<SpectralVolumeData>> _subVolumes;
};

template <class... Volumes>
CompositeVolume::CompositeVolume(SpectralVolumeData volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.emplace_front(new SpectralVolumeData(std::move(volume)));
}

template <class... Volumes>
CompositeVolume::CompositeVolume(std::unique_ptr<SpectralVolumeData> volume,
                                 Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.push_front(std::move(volume));
}

template <class... Volumes>
CompositeVolume::CompositeVolume(const AbstractDynamicVolumeData& volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.emplace_front(volume.clone());
}

} // namespace CTL

#endif // COMPOSITEVOLUME_H
