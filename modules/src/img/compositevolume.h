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

    // getter methods
    const SpectralVolumeData& subVolume(uint materialIdx) const;
    std::unique_ptr<SpectralVolumeData> muVolume(uint materialIdx, float centerEnergy,
                                                 float binWidth) const;
    uint nbSubVolumes() const;

    // other methods
    void addSubVolume(SpectralVolumeData volume);
    void addSubVolume(std::unique_ptr<SpectralVolumeData> volume);
    void addSubVolume(const AbstractDynamicVolumeData& volume);

private:
    std::deque<std::unique_ptr<SpectralVolumeData>> _subVolumes;
};

template <class... Volumes>
CompositeVolume::CompositeVolume(SpectralVolumeData volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.push_front(
        std::unique_ptr<SpectralVolumeData>(new SpectralVolumeData(std::move(volume))));
}

} // namespace CTL

#endif // COMPOSITEVOLUME_H
