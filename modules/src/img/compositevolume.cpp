#include "compositevolume.h"

namespace CTL {

// ### CompositeVolume ###

SpectralVolumeData& CompositeVolume::subVolume(uint volIdx)
{
    return *_subVolumes[volIdx];
}

/*!
 * Returns the sub-volume at position \a volIdx.
 *
 * Does not perform out-of-range checks.
 */
const SpectralVolumeData& CompositeVolume::subVolume(uint volIdx) const
{
    return *_subVolumes[volIdx];
}

/*!
 * Convenience method; returns the sub-volume at position \a volIdx transformed to attenuation
 * values corresponding to the energy bin specified by \a centerEnergy and \a binWidth.
 * Same as: subVolume(volIdx).muVolume(centerEnergy, binWidth)
 *
 * Does not perform out-of-range checks.
 *
 * \sa SpectralVolumeData::muVolume().
 */
std::unique_ptr<SpectralVolumeData>
CompositeVolume::muVolume(uint volIdx, float centerEnergy, float binWidth) const
{
    return _subVolumes[volIdx]->muVolume(centerEnergy, binWidth);
}

uint CompositeVolume::nbSubVolumes() const { return static_cast<uint>(_subVolumes.size()); }

const std::deque<CompositeVolume::SubVolPtr>& CompositeVolume::data() const
{
    return _subVolumes;
}

std::deque<CompositeVolume::SubVolPtr>& CompositeVolume::data()
{
    return _subVolumes;
}

bool CompositeVolume::isEmpty() const
{
    return _subVolumes.empty();
}

void CompositeVolume::addSubVolume(SpectralVolumeData volume)
{
    _subVolumes.emplace_back(new SpectralVolumeData(std::move(volume)));
}

void CompositeVolume::addSubVolume(std::unique_ptr<SpectralVolumeData> volume)
{
    _subVolumes.emplace_back(std::move(volume));
}

void CompositeVolume::addSubVolume(const AbstractDynamicVolumeData& volume)
{
    _subVolumes.emplace_back(volume.clone());
}

void CompositeVolume::addSubVolume(CompositeVolume&& volume)
{
    std::for_each(volume._subVolumes.begin(), volume._subVolumes.end(),
                  [this](SubVolPtr& vol) {
                      _subVolumes.push_back(std::move(vol));
                  });
}

void CompositeVolume::addSubVolume(const CompositeVolume& volume)
{
    std::for_each(volume._subVolumes.cbegin(), volume._subVolumes.cend(),
                  [this](const SubVolPtr& vol) {
                      _subVolumes.emplace_back(vol->clone());
                  });
}

} // namespace CTL
