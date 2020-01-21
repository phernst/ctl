#ifndef COMPOSITEVOLUME_H
#define COMPOSITEVOLUME_H

#include "spectralvolumedata.h"

namespace CTL {

class CompositeVolume
{
public:

    // getter methods
    const SpectralVolumeData& materialVolume(uint materialIdx) const;
    SpectralVolumeData muVolume(uint materialIdx, float centerEnergy, float binWidth) const;
    uint nbMaterials() const;

    // other methods
    void addMaterialVolume(const SpectralVolumeData& volume);
    void addMaterialVolume(SpectralVolumeData&& volume);

private:
    std::vector<SpectralVolumeData> _materialVolumes;
};

}

#endif // COMPOSITEVOLUME_H
