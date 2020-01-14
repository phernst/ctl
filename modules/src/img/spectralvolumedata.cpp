#include "spectralvolumedata.h"
#include "io/ctldatabase.h"
#include <cmath>

namespace CTL {

SpectralVolumeData::SpectralVolumeData(const VoxelVolume<float>& materialDensity,
                                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                       const QString& materialName)
    : VoxelVolume<float> (materialDensity)
    , _absorptionModel(std::move(absorptionModel))
    , _materialName(materialName)
{
}

SpectralVolumeData::SpectralVolumeData(VoxelVolume<float>&& materialDensity,
                                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                       const QString& materialName)
    : VoxelVolume<float> (std::move(materialDensity))
    , _absorptionModel(std::move(absorptionModel))
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
        qWarning("SpectralVolumeData::averageMassAttenuationFactor: Interval width is zero!"
                 "Delegating call to SpectralVolumeData::massAttenuationFactor.");
        return massAttenuationFactor(centerEnergy);
    }

    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::averageMassAttenuationFactor: No absorption model set!");

    return _absorptionModel->binIntegral(centerEnergy, binWidth) / binWidth;
}

const VoxelVolume<float>& SpectralVolumeData::density() const
{
    return *this;
}

float SpectralVolumeData::massAttenuationFactor(float atEnergy) const
{
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::massAttenuationFactor: No absorption model set!");

    return _absorptionModel->valueAt(atEnergy);
}

const QString& SpectralVolumeData::materialName() const
{
    return _materialName;
}

VoxelVolume<float> SpectralVolumeData::muVolume(float centerEnergy, float binWidth) const
{
    constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm
    return (*this) * (averageMassAttenuationFactor(centerEnergy, binWidth) * cm2mm);
}

SpectralVolumeData
SpectralVolumeData::createBall(float radius,
                               float voxelSize,
                               float density,
                               std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    const auto nbVox = static_cast<uint>(std::ceil(2.0f * radius / voxelSize));
    const auto center = float(nbVox - 1) / 2.0f;

    const SpectralVolumeData::Dimensions volDim{ nbVox, nbVox, nbVox };
    const SpectralVolumeData::VoxelSize voxSize{ voxelSize, voxelSize, voxelSize };
    SpectralVolumeData volume{ { volDim, voxSize }, std::move(absorptionModel) };
    volume.fill(density);

    auto dist2Center = [center](float x, float y, float z)
    {
        const auto dx = x - center;
        const auto dy = y - center;
        const auto dz = z - center;
        return dx * dx + dy * dy + dz * dz;
    };
    const auto rSquaredInVoxel = (radius / voxelSize) * (radius / voxelSize);

    // erase exterior space
    for(auto x = 0u; x < nbVox; ++x)
        for(auto y = 0u; y < nbVox; ++y)
            for(auto z = 0u; z < nbVox; ++z)
                if(dist2Center(x, y, z) > rSquaredInVoxel)
                    volume(x, y, z) = 0.0f;

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

SpectralVolumeData SpectralVolumeData::fromMuVolume(VoxelVolume<float> muValues,
                                                    std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                                    float referenceEnergy)
{
    // transform to densities (g/cm^3)
    muValues /= (0.1f * absorptionModel->valueAt(referenceEnergy));

    return SpectralVolumeData(std::move(muValues), absorptionModel, absorptionModel->name());
}

SpectralVolumeData SpectralVolumeData::fromHUVolume(VoxelVolume<float> HUValues,
                                                    std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                                    float referenceEnergy)
{
    // transform to attenuation values
    auto muWater = database::attenuationModel(database::Composite::Water)->valueAt(referenceEnergy);
    HUValues = (HUValues*(muWater/1000.0f)) + muWater;

    // transform to densities (g/cm^3)
    HUValues /= (0.1f * absorptionModel->valueAt(referenceEnergy));

    return SpectralVolumeData(std::move(HUValues), absorptionModel, absorptionModel->name());
}

} // namespace CTL
