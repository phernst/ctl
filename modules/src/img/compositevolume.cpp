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
 *
 * Example:
 * \code
 * CompositeVolume volume;
 *
 * // add a water cube
 * volume.addSubVolume(SpectralVolumeData::cube(50, 1.0f, 1.0f, attenuationModel(database::Composite::Water)));
 *
 * // we can also add a plain VoxelVolume (i.e. representing attenuation coefficients)
 * volume.addSubVolume(VoxelVolume<float>::ball(20.0f, 1.0f, 0.05f)); // this uses implicit cast to SpectralVolumeData
 * \endcode
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
 *
 * Example:
 * \code
 * CompositeVolume volume;
 *
 * // define a simple dynamic volume: cube that increases in attenuation by 0.01/mm per millisecond
 * LinearDynamicVolume dynamicVol(0.01f, 0.0f, { 100, 100, 100 }, { 1.0f, 1.0f, 1.0f });
 *
 * // add it as sub-volume
 * volume.addSubVolume(dynamicVol);
 * \endcode
 */
void CompositeVolume::addSubVolume(const AbstractDynamicVolumeData& volume)
{
    _subVolumes.emplace_back(volume.clone());
}

/*!
 * Adds all sub-volumes of \a volume to this instance.
 *
 * Example:
 * \code
 * CompositeVolume volume;  // our final volume
 *
 * // We first add a water cube to the volume
 * volume.addSubVolume(SpectralVolumeData::cube(100, 1.0f, 1.0f, attenuationModel(database::Composite::Water)));
 *
 * // We now add another composite, consisting of two testicle balls.
 * volume.addSubVolume(CompositeVolume(SpectralVolumeData::ball(15.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Testis)),
 *                                     SpectralVolumeData::ball(15.0f, 1.0f, 1.1f, attenuationModel(database::Composite::Testis))));
 *
 * // If necessary, we can now shift individual sub-volumes slightly around the center, e.g. like that:
 * volume.subVolume(1).setVolumeOffset(-20.0f, 0.0f, 0.0f); // shifts ball 1 towards a negative x position
 * volume.subVolume(2).setVolumeOffset( 20.0f, 0.0f, 0.0f); // shifts ball 2 towards a positive x position
 * \endcode
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
 *
 * Example:
 * \code
 * CompositeVolume volume;  // our final volume
 *
 * // We first add a water cube to the volume
 * volume.addSubVolume(SpectralVolumeData::cube(100, 1.0f, 1.0f, attenuationModel(database::Composite::Water)));
 *
 * // We now construct another composite, consisting of two testicle balls.
 * CompositeVolume subComposite {
 *     SpectralVolumeData::ball(15.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Testis)),
 *     SpectralVolumeData::ball(15.0f, 1.0f, 1.1f, attenuationModel(database::Composite::Testis))
 * };
 *
 * // We shift both sub-volumes slightly around the center.
 * subComposite.subVolume(0).setVolumeOffset(-20.0f, 0.0f, 0.0f);
 * subComposite.subVolume(1).setVolumeOffset( 20.0f, 0.0f, 0.0f);
 *
 * // Now, we can add the composite containing the two balls to our final volume (which already holds the water cube).
 * volume.addSubVolume(subComposite);
 * \endcode
 */
void CompositeVolume::addSubVolume(const CompositeVolume& volume)
{
    std::for_each(volume._subVolumes.cbegin(), volume._subVolumes.cend(),
                  [this](const SubVolPtr& vol) {
                      _subVolumes.emplace_back(vol->clone());
                  });
}

} // namespace CTL
