#ifndef COMPOSITEVOLUME_H
#define COMPOSITEVOLUME_H

#include "abstractdynamicvolumedata.h"
#include <deque>

namespace CTL {
/*!
 * \class CompositeVolume
 *
 * \brief The CompositeVolume class is a container to hold multiple volume datasets of any type from
 * the CTL.
 *
 * This class can hold multiple volume datasets of SpectralVolumeData or its subclasses.
 * More precisely, an instance of CompositeVolume can consume (copy or move)
 * - SpectralVolumeData
 * - a VoxelVolume (by means of implicit conversion to SpectralVolumeData)
 * - any implementation of AbstractDynamicVolumeData (only by copy or by an unique_ptr)
 * - another CompositeVolume (collecting its sub-volumes individually).
 *
 * When used with a projector, the CompositeVolume object must be passed to
 * AbstractProjector::projectComposite(). This results in computation of projections considering all
 * sub-volumes held by the CompositeVolume object (with all their individual properties, such as
 * spectral information or temporal dynamics; given that appropriate projector extensions are in
 * use).
 *
 * Sub-volume are added to the container using addSubVolume(). Alternatively, the CompositeVolume
 * can be created directly using a constructor and passing to it all sub-volumes that shall be
 * added.
 *
 * All sub-volumes may differ in any arbitrary property, for example:
 * - dimensions (i.e. voxel count)
 * - voxel size
 * - (positional) offset
 * - volume type (plain, spectral, dynamic)
 * This allows for fully flexible composition of phantom data.
 */
class CompositeVolume
{
public:
    using SubVolPtr = CopyableUniquePtr<SpectralVolumeData>;

    CompositeVolume() = default;
    template <class... Volumes>
    CompositeVolume(SpectralVolumeData volume, Volumes&&... otherVolumes);
    template <class... Volumes>
    CompositeVolume(std::unique_ptr<SpectralVolumeData> volume, Volumes&&... otherVolumes);
    template <class... Volumes>
    CompositeVolume(const AbstractDynamicVolumeData& volume, Volumes&&... otherVolumes);
    template <class... Volumes>
    CompositeVolume(CompositeVolume&& volume, Volumes&&... otherVolumes);
    template <class... Volumes>
    CompositeVolume(const CompositeVolume& volume, Volumes&&... otherVolumes);

    CompositeVolume(const CompositeVolume& volume) = default;
    CompositeVolume(CompositeVolume&& volume) = default;
    CompositeVolume& operator=(const CompositeVolume& volume) = default;
    CompositeVolume& operator=(CompositeVolume&& volume) = default;
    ~CompositeVolume() = default;

    // getter methods
    const std::deque<SubVolPtr>& data() const;
    std::deque<SubVolPtr>& data();
    bool isEmpty() const;
    std::unique_ptr<SpectralVolumeData> muVolume(uint volIdx, float centerEnergy,
                                                 float binWidth) const;
    uint nbSubVolumes() const;
    const SpectralVolumeData& subVolume(uint volIdx) const;
    SpectralVolumeData& subVolume(uint volIdx);

    // other methods
    void addSubVolume(SpectralVolumeData volume);
    void addSubVolume(std::unique_ptr<SpectralVolumeData> volume);
    void addSubVolume(const AbstractDynamicVolumeData& volume);
    void addSubVolume(CompositeVolume&& volume);
    void addSubVolume(const CompositeVolume& volume);

private:
    std::deque<SubVolPtr> _subVolumes;
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
    _subVolumes.emplace_front(std::move(volume));
}

template <class... Volumes>
CompositeVolume::CompositeVolume(const AbstractDynamicVolumeData& volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.emplace_front(volume.clone());
}

template <class... Volumes>
CompositeVolume::CompositeVolume(CompositeVolume&& volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    std::for_each(volume._subVolumes.begin(), volume._subVolumes.end(),
                  [this](SubVolPtr& vol) {
                      _subVolumes.push_front(std::move(vol));
                  });
}

template <class... Volumes>
CompositeVolume::CompositeVolume(const CompositeVolume& volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    std::for_each(volume._subVolumes.cbegin(), volume._subVolumes.cend(),
                  [this](const SubVolPtr& vol) {
                      _subVolumes.emplace_front(vol->clone());
                  });
}

} // namespace CTL

#endif // COMPOSITEVOLUME_H
