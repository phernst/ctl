#include "compositevolume.h"
#include <cmath>

namespace CTL {

SpectralVolumeData::SpectralVolumeData(const VoxelVolume<float>& materialDensity,
                                         std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                         const QString& materialName)
    : VoxelVolume<float> (materialDensity)
    , _absorptionModel(absorptionModel)
    , _materialName(materialName)
{
}

SpectralVolumeData::SpectralVolumeData(VoxelVolume<float>&& materialDensity,
                                         std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                         const QString& materialName)
    : VoxelVolume<float> (std::move(materialDensity))
    , _absorptionModel(absorptionModel)
    , _materialName(materialName)
{
}

std::shared_ptr<AbstractIntegrableDataModel> SpectralVolumeData::absorptionModel() const
{
    return _absorptionModel;
}

float SpectralVolumeData::averageMassAttenuationFactor(float centerEnergy, float binWidth) const
{
    if(qFuzzyIsNull(binWidth))
    {
        qWarning("RealisticVolumeData::averageMassAttenuationFactor: Interval width is zero!"
                 "Delegating call to RealisticVolumeData::massAttenuationFactor.");
        return massAttenuationFactor(centerEnergy);
    }

    if(!_absorptionModel)
    {
        qWarning("RealisticVolumeData::averageMassAttenuationFactor: No absorption model set!");
        return -1.0f;
    }

    return _absorptionModel->binIntegral(centerEnergy, binWidth) / binWidth;
}

const VoxelVolume<float>& SpectralVolumeData::density() const
{
    return *this;
}

float SpectralVolumeData::massAttenuationFactor(float atEnergy) const
{
    if(!_absorptionModel)
    {
        qWarning("RealisticVolumeData::massAttenuationFactor: No absorption model set!");
        return -1.0f;
    }

    return _absorptionModel->valueAt(atEnergy);
}

const QString& SpectralVolumeData::materialName() const
{
    return _materialName;
}

VoxelVolume<float> SpectralVolumeData::muVolume(float centerEnergy, float binWidth) const
{
    return (*this) * (averageMassAttenuationFactor(centerEnergy, binWidth) * 0.1f);
}

SpectralVolumeData SpectralVolumeData::createBall(float radius, float voxelSize, float density,
                                                  std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    uint nbVox = static_cast<uint>(ceil(2.0f * radius / voxelSize));

    SpectralVolumeData::Dimensions volDim{ nbVox, nbVox, nbVox };
    SpectralVolumeData::VoxelSize voxSize = { voxelSize, voxelSize, voxelSize };
    SpectralVolumeData volume({volDim, voxSize});
    volume.fill(density);
    volume.setAbsorptionModel(absorptionModel);

    float center = float(nbVox-1)/2.0;
    float rSquaredInVoxel = radius/voxelSize * radius/voxelSize ;

    for(float x = 0.0f; x < nbVox; ++x)
        for(float y = 0.0f; y < nbVox; ++y)
            for(float z = 0.0f; z < nbVox; ++z)
                if((x-center)*(x-center) + (y-center)*(y-center) + (z-center)*(z-center) > rSquaredInVoxel)
                    volume(x,y,z) = 0.0f;

    return volume;
}

void SpectralVolumeData::setAbsorptionModel(AbstractIntegrableDataModel* absorptionModel)
{
    _absorptionModel = std::shared_ptr<AbstractIntegrableDataModel>(absorptionModel);
}

void SpectralVolumeData::setAbsorptionModel(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    _absorptionModel = absorptionModel;
}

void SpectralVolumeData::setDensity(const VoxelVolume<float>& density)
{
    *this = density;
}

void SpectralVolumeData::setDensity(VoxelVolume<float>&& density)
{
    *this = std::move(density);
}

void SpectralVolumeData::setMaterialName(const QString& name)
{
    _materialName = name;
}

// ### CompositeVolume ###

const SpectralVolumeData& CompositeVolume::materialVolume(uint materialIdx) const
{
    return _materialVolumes[materialIdx];
}

VoxelVolume<float> CompositeVolume::muVolume(uint materialIdx, float centerEnergy, float binWidth) const
{
    return _materialVolumes[materialIdx].muVolume(centerEnergy, binWidth);
}

uint CompositeVolume::nbMaterials() const
{
    return static_cast<uint>(_materialVolumes.size());
}

void CompositeVolume::addMaterialVolume(const SpectralVolumeData& volume)
{
    _materialVolumes.push_back(volume);
}

void CompositeVolume::addMaterialVolume(SpectralVolumeData&& volume)
{
    _materialVolumes.push_back(std::move(volume));
}

}
