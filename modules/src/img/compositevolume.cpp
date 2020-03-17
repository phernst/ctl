#include "compositevolume.h"

namespace CTL {

// ### CompositeVolume ###
/*!
 * Returns a (modifiable) reference to the sub-volume at position \a volIdx.
 *
 * Does not perform out-of-range checks.
 */
SpectralVolumeData& CompositeVolume::subVolume(uint volIdx)
{
    return *_subVolumes[volIdx];
}

/*!
 * Returns a constant reference to the sub-volume at position \a volIdx.
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

/*!
 * Returns the number of sub-volumes in this instance.
 */
uint CompositeVolume::nbSubVolumes() const { return static_cast<uint>(_subVolumes.size()); }

/*!
 * Returns a constant reference to the data managed by this instance.
 */
const std::deque<CompositeVolume::SubVolPtr>& CompositeVolume::data() const
{
    return _subVolumes;
}

/*!
 * Returns a (modifiably) reference to the data managed by this instance.
 */
std::deque<CompositeVolume::SubVolPtr>& CompositeVolume::data()
{
    return _subVolumes;
}

/*!
 * Returns true if this instance has no sub-volumes.
 */
bool CompositeVolume::isEmpty() const
{
    return _subVolumes.empty();
}

/*!
 * Adds \a volume as aub-volume to this instance.
 */
void CompositeVolume::addSubVolume(SpectralVolumeData volume)
{
    _subVolumes.emplace_back(new SpectralVolumeData(std::move(volume)));
}

/*!
 * Adds \a volume as aub-volume to this instance.
 */
void CompositeVolume::addSubVolume(std::unique_ptr<SpectralVolumeData> volume)
{
    _subVolumes.emplace_back(std::move(volume));
}

/*!
 * Adds \a volume as aub-volume to this instance. The volume will be cloned.
 */
void CompositeVolume::addSubVolume(const AbstractDynamicVolumeData& volume)
{
    _subVolumes.emplace_back(volume.clone());
}

/*!
 * Adds all sub-volumes of \a volume to this instance.
 */
void CompositeVolume::addSubVolume(CompositeVolume&& volume)
{
    std::for_each(volume._subVolumes.begin(), volume._subVolumes.end(),
                  [this](SubVolPtr& vol) {
                      _subVolumes.push_back(std::move(vol));
                  });
}

/*!
 * Adds all sub-volumes of \a volume to this instance. All sub-volumes will be cloned.
 */
void CompositeVolume::addSubVolume(const CompositeVolume& volume)
{
    std::for_each(volume._subVolumes.cbegin(), volume._subVolumes.cend(),
                  [this](const SubVolPtr& vol) {
                      _subVolumes.emplace_back(vol->clone());
                  });
}

} // namespace CTL
