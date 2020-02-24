#ifndef SPECTRALVOLUMEDATA_H
#define SPECTRALVOLUMEDATA_H

#include "voxelvolume.h"
#include "models/abstractdatamodel.h"

namespace CTL {
/*!
 * \class SpectralVolumeData
 *
 * \brief The SpectralVolumeData class holds voxelized data (for a single material) along with
 * information on its spectrally-dependent absorption properties.
 *
 * This class is an extension of the VoxelVolume class (template type float) by means of enhancing
 * it with information on its spectrally-dependent absorption properties. It represents a single
 * material (either elemental or composite), in the way that the spectral dependency of the mass
 * attenuation coefficient is assumed identical for all voxels (use CompositeVolume in cases where
 * this assumption is too restrictive).
 *
 * Spectral information always requires a data model describing the spectral dependency of the
 * mass-attenuation coefficient for the material represented by this volume. Additionally, one of
 * the following conditions must be fulfilled:
 * - numerical values in individual voxels represent material density (in g/cm^-3), or
 * - numerical values in individual voxels represent attenuation coefficiens (in 1/mm) and the
 * corresponding reference energy is specified.
 *
 * The two different representations can be queried explicitely with densityVolume() and muVolume().
 * Note that both these methods need to create copies of the data.
 *
 * Availability of spectral information in the object can be checked via hasSpectralInformation().
 * In case the abovementioned conditions are not entirely fulfilled, the stored data is considered
 * to represent straightforward attenuation coefficients (mu, in 1/mm) without any spectral
 * information. Internally, such a volume object works with a constant attenuation model (i.e.
 * mu/rho = 1.0 cm^2/g across entire energy range). This behavior is implemented only for
 * convenience and it is recommended to use plain VoxelVolume<float> objects for such cases.
 * Any VoxelVolume<float> object representing attenuation values (either in 1/mm (i.e. mu) or
 * Hounsfield units) can be transformed into SpectralVolumeData using the factory methods
 * fromMuVolume() or fromHUVolume() to complement the missing spectral information.
 */
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
    bool isDensityVolume() const;
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

    bool _hasNonDefaultAbsModel{ false };    //!< True if absorption model has been set explicitely.
    bool _isMu{ false };                     //!< True if voxel data represents att. coefficents.
    float _refEnergy{ -1.0f };               //!< Reference energy corresponding to mu values.
    float _refMassAttenuationCoeff{ -1.0f }; //!< Reference att. coeff. corresponding to mu values.

    // other methods
    void changeReferenceEnergy(float newReferenceEnergy);
    void changeReferenceMassAttCoeff(float newReferenceMassAttCoeff, float correspondingRefEnergy);
    void transformToAttenuationCoeff(float referenceEnergy);
    void transformToAttenuationCoeff(float referenceMassAttCoeff, float correspondingRefEnergy);
    void transformToDensity();
};

}

#endif // SPECTRALVOLUMEDATA_H
