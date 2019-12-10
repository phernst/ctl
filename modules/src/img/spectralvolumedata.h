#ifndef SPECTRALVOLUMEDATA_H
#define SPECTRALVOLUMEDATA_H

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

    static SpectralVolumeData createBall(float radius, float voxelSize, float density,
                                         std::shared_ptr<AbstractIntegrableDataModel> absorptionModel = nullptr);

    // setter methods
    void setAbsorptionModel(AbstractIntegrableDataModel* absorptionModel);
    void setAbsorptionModel(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel);
    void setDensity(const VoxelVolume<float>& density);
    void setDensity(VoxelVolume<float>&& density);
    void setMaterialName(const QString& name);

    using VoxelVolume<float>::operator=;

    // factory methods
    static SpectralVolumeData fromMuVolume(VoxelVolume<float> muValues,
                                           std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                           float referenceEnergy = 50.0f);
    static SpectralVolumeData fromHUVolume(VoxelVolume<float> HUValues,
                                           std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                           float referenceEnergy = 50.0f);

private:
    std::shared_ptr<AbstractIntegrableDataModel> _absorptionModel;
    QString _materialName;
};

}

#endif // SPECTRALVOLUMEDATA_H
