#ifndef CTL_COMPOSITEVOLUME_H
#define CTL_COMPOSITEVOLUME_H

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
 *
 * The following code example shows one possibility to construct a volume consisting of a cube
 * filled with extinction value 0.02/mm and two balls, a smaller one with 0.05/mm and a slightly
 * larger one with 0.1/mm. We want the first ball to be positioned slightly in negative y-direction and
 * the second one in the opposite direction.
 * \code
 * // Construct the composite volume already containing the cube volume.
 * CompositeVolume volume(VoxelVolume<float>::cube(150, 1.0f, 0.02f));
 *
 * // We now construct the two ball volumes.
 * auto subVolume1 = VoxelVolume<float>::ball(10.0f, 1.0f, 0.05f);
 * auto subVolume2 = VoxelVolume<float>::ball(25.0f, 1.0f, 0.10f);
 *
 * // Here, we shift the ball volumes to the desired positions.
 * subVolume1.setVolumeOffset(0.0f, -20.0f, 0.0f);
 * subVolume2.setVolumeOffset(0.0f,  30.0f, 0.0f);
 *
 * // Now, we add the two balls as sub-volumes to our final volume.
 * volume.addSubVolume(std::move(subVolume1));
 * volume.addSubVolume(std::move(subVolume2));
 * \endcode
 *
 * We can now create a projection image from this composite with the following example code:
 * \code
 * // First, we need to define an acquisition setup (with a CT system and the number of views; 10 in this case)
 * AcquisitionSetup setup(CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT(DetectorBinning::Binning4x4)), 10);
 * // We also need to specify the acquisition geometry, here we set a simple short scan trajectory
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0f));
 *
 * // Now, we create our projector, here we simply use the standard pipeline.
 * auto projector = makeProjector<StandardPipeline>();
 *
 * // Pass the acquisition setup to the projector and create the projections:
 * projector->configure(setup);
 * auto projections = projector->projectComposite(volume);
 * \endcode
 *
 * ![First view of the projections generated with the above example.](compositeVolume.png)
 *
 * The same can be done with spectral volumes as well. It is also possible to mix both types as
 * shown in the following example. Note, however, that these mixtures are currently (v. 0.3.1)
 * only supported in the non-linear case of SpectralEffectsExtension (which is used for example
 * with the StandardPipeline in No_Approximation preset).
 * \code
 * // Our starting volume remains the same. This is the sub-volume without spectral information.
 * CompositeVolume volume(VoxelVolume<float>::cube(150, 1.0f, 0.02f));
 *
 * // We now create two balls with spectral information (one representing blood, the other bone).
 * auto subVolume1 = SpectralVolumeData::ball(10.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Blood));
 * auto subVolume2 = SpectralVolumeData::ball(25.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Bone_Cortical));
 *
 * // Again, the shift to the desired positions...
 * subVolume1.setVolumeOffset(0.0f, -20.0f, 0.0f);
 * subVolume2.setVolumeOffset(0.0f,  30.0f, 0.0f);
 *
 * // ... and adding to the final volume.
 * volume.addSubVolume(std::move(subVolume1));
 * volume.addSubVolume(std::move(subVolume2));
 *
 * // In the projection code, we only change the setting for the standard pipeline to 'No_Approximation'...
 * AcquisitionSetup setup(CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT(DetectorBinning::Binning4x4)), 10);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0f));
 *
 * //... here comes the changed line:
 * auto projector = makeProjector<StandardPipeline>(StandardPipeline::No_Approximation);
 *
 * projector->configure(setup);
 * auto projections = projector->projectComposite(volume);
 * \endcode
 *
 * ![First view of the projections generated with the example for mixed volumes.](compositeVolume_mixed.png)
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

/*!
 * Constructs a CompositeVolume and adds all data passed to the constructor as sub-volumes.
 *
 * Example:
 * \code
 * // pass a water cube
 * CompositeVolume volume(SpectralVolumeData::cube(50, 1.0f, 1.0f, attenuationModel(database::Composite::Water)));
 *
 * // we can also pass a plain VoxelVolume (i.e. representing attenuation coefficients)
 * CompositeVolume volume2(VoxelVolume<float>::ball(20.0f, 1.0f, 0.05f)); // this uses implicit cast to SpectralVolumeData
 * \endcode
 */
template <class... Volumes>
CompositeVolume::CompositeVolume(SpectralVolumeData volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.emplace_front(new SpectralVolumeData(std::move(volume)));
}

/*!
 * Constructs a CompositeVolume and adds all data passed to the constructor as sub-volumes.
 */
template <class... Volumes>
CompositeVolume::CompositeVolume(std::unique_ptr<SpectralVolumeData> volume,
                                 Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.emplace_front(std::move(volume));
}

/*!
 * Constructs a CompositeVolume and adds all data passed to the constructor as sub-volumes.
 *
 * Example:
 * \code
 * // pass a simple dynamic volume: cube that increases in attenuation by 0.01/mm per millisecond
 * CompositeVolume volume(LinearDynamicVolume(0.01f, 0.0f, { 100, 100, 100 }, { 1.0f, 1.0f, 1.0f }));
 * \endcode
 */
template <class... Volumes>
CompositeVolume::CompositeVolume(const AbstractDynamicVolumeData& volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    _subVolumes.emplace_front(volume.clone());
}

/*!
 * Constructs a CompositeVolume and adds all data passed to the constructor as sub-volumes.
 *
 * Example:
 * \code
 * // Passing a composite consisting of two balls as well as a single cube volume:
 * CompositeVolume volume(CompositeVolume {
 *                            SpectralVolumeData::ball(15.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Blood)),
 *                            SpectralVolumeData::ball(15.0f, 1.0f, 1.1f, attenuationModel(database::Composite::Blood))
 *                        },
 *                        SpectralVolumeData::cube(100, 1.0f, 1.0f, attenuationModel(database::Composite::Water)));
 * \endcode
 */
template <class... Volumes>
CompositeVolume::CompositeVolume(CompositeVolume&& volume, Volumes&&... otherVolumes)
    : CompositeVolume(std::forward<Volumes>(otherVolumes)...)
{
    std::for_each(volume._subVolumes.begin(), volume._subVolumes.end(),
                  [this](SubVolPtr& vol) {
                      _subVolumes.push_front(std::move(vol));
                  });
}

/*!
 * Constructs a CompositeVolume and adds all data passed to the constructor as sub-volumes.
 *
 * Example:
 * \code
 * // Passing a composite consisting of two balls as well as a single cube volume:
 * CompositeVolume volume(CompositeVolume {
 *                            SpectralVolumeData::ball(15.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Blood)),
 *                            SpectralVolumeData::ball(15.0f, 1.0f, 1.1f, attenuationModel(database::Composite::Blood))
 *                        },
 *                        SpectralVolumeData::cube(100, 1.0f, 1.0f, attenuationModel(database::Composite::Water)));
 * \endcode
 */
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

#endif // CTL_COMPOSITEVOLUME_H
