#ifndef COMPOSITEVOLUME_H
#define COMPOSITEVOLUME_H

#include "voxelvolume.h"
#include "models/abstractdatamodel.h"

namespace CTL {

class SpectralVolumeData : public VoxelVolume<float>
{
public:
    SpectralVolumeData(const VoxelVolume<float>& materialDensity,
                        std::shared_ptr<AbstractIntegrableDataModel> absorptionModel = nullptr,
                        const QString& materialName = "NN");
    SpectralVolumeData(VoxelVolume<float>&& materialDensity,
                        std::shared_ptr<AbstractIntegrableDataModel> absorptionModel = nullptr,
                        const QString& materialName = "NN");

    // getter methods
    std::shared_ptr<AbstractIntegrableDataModel> absorptionModel() const;
    float averageMassAttenuationFactor(float centerEnergy, float binWidth) const;
    const VoxelVolume<float>& density() const;
    float massAttenuationFactor(float atEnergy) const;
    const QString& materialName() const;
    VoxelVolume<float> muVolume(float centerEnergy, float binWidth) const;

    // setter methods
    void setAbsorptionModel(AbstractIntegrableDataModel* absorptionModel);
    void setAbsorptionModel(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel);
    void setDensity(const VoxelVolume<float>& density);
    void setDensity(VoxelVolume<float>&& density);
    void setMaterialName(const QString& name);

    using VoxelVolume<float>::operator=;

private:
    //VoxelVolume<float> _materialDensity;
    std::shared_ptr<AbstractIntegrableDataModel> _absorptionModel;
    QString _materialName;
};

class CompositeVolume
{
public:

    // getter methods
    const SpectralVolumeData& materialVolume(uint materialIdx) const;
    VoxelVolume<float> muVolume(uint materialIdx, float centerEnergy, float binWidth) const;
    uint nbMaterials() const;

    // other methods
    void addMaterialVolume(const SpectralVolumeData& volume);
    void addMaterialVolume(SpectralVolumeData&& volume);

private:
    std::vector<SpectralVolumeData> _materialVolumes;
};

}

#endif // COMPOSITEVOLUME_H
