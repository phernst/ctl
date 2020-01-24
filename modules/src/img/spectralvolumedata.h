#ifndef SPECTRALVOLUMEDATA_H
#define SPECTRALVOLUMEDATA_H

#include "voxelvolume.h"
#include "models/abstractdatamodel.h"

namespace CTL {

class SpectralVolumeData : public VoxelVolume<float>
{
public:
    SpectralVolumeData(VoxelVolume<float> muValues);
    SpectralVolumeData(VoxelVolume<float> muValues,
                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                       float referenceEnergy,
                       const QString& materialName = QString());
    SpectralVolumeData(VoxelVolume<float> materialDensity,
                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                       const QString& materialName = QString());

    // getter methods
    std::shared_ptr<AbstractIntegrableDataModel> absorptionModel() const;
    SpectralVolumeData densityVolume() const;
    bool hasSpectralInformation() const;
    bool isMuVolume() const;
    float massAttenuationCoeff(float atEnergy) const;
    const QString& materialName() const;
    float meanMassAttenuationCoeff(float centerEnergy, float binWidth) const;
    SpectralVolumeData muVolume(float referenceEnergy) const;
    SpectralVolumeData muVolume(float centerEnergy, float binWidth) const;
    float referenceEnergy() const;
    float referenceMassAttenuationCoeff() const;

    static SpectralVolumeData createBall(float radius, float voxelSize, float muValue);
    static SpectralVolumeData createBall(float radius, float voxelSize, float density,
                                         std::shared_ptr<AbstractIntegrableDataModel> absorptionModel);

    // setter methods
    void replaceAbsorptionModel(AbstractIntegrableDataModel* absorptionModel);
    void replaceAbsorptionModel(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel);
    void setDensity(VoxelVolume<float> density);
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

    bool _isMu = false;         //!< True if voxel data represents absorption coefficents.
    float _refEnergy = -1.0f;   //!< Reference energy corresponding to mu values.
    float _refMassAttenuationCoeff{-1.0f}; //!< Reference attenuation coeff. corresponding to mu values.

    // other methods
    void changeReferenceEnergy(float newReferenceEnergy);
    void changeReferenceMassAttCoeff(float newReferenceMassAttCoeff, float correspondingRefEnergy);
    void transformToAttenuationCoeff(float referenceEnergy);
    void transformToAttenuationCoeff(float referenceMassAttCoeff, float correspondingRefEnergy);
    void transformToDensity();
};

}

#endif // SPECTRALVOLUMEDATA_H
