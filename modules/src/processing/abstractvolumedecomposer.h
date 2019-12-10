#ifndef ABSTRACTVOLUMEDECOMPOSER_H
#define ABSTRACTVOLUMEDECOMPOSER_H

#include "img/compositevolume.h"

namespace CTL {

class AbstractVolumeDecomposer
{
public:
    virtual CompositeVolume decompose(const VoxelVolume<float>& volume,
                                      float referenceEnergy = 50.0f) const = 0;
    virtual ~AbstractVolumeDecomposer() = default;

protected:
    AbstractVolumeDecomposer() = default;
    AbstractVolumeDecomposer(const AbstractVolumeDecomposer&) = default;
    AbstractVolumeDecomposer(AbstractVolumeDecomposer&&) = default;
    AbstractVolumeDecomposer& operator=(const AbstractVolumeDecomposer&) = default;
    AbstractVolumeDecomposer& operator=(AbstractVolumeDecomposer&&) = default;
};

} // namespace CTL

#endif // ABSTRACTVOLUMEDECOMPOSER_H
