#include "compositevolume.h"

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

    return _absorptionModel->binIntegral(centerEnergy, binWidth) / binWidth;
}

const VoxelVolume<float>& SpectralVolumeData::density() const
{
    return *this;
}

float SpectralVolumeData::massAttenuationFactor(float atEnergy) const
{
    return _absorptionModel->valueAt(atEnergy);
}

const QString& SpectralVolumeData::materialName() const
{
    return _materialName;
}

VoxelVolume<float> SpectralVolumeData::muVolume(float centerEnergy, float binWidth) const
{
    return (*this) * averageMassAttenuationFactor(centerEnergy, binWidth);
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
