#include "spectralvolumedata.h"
#include "io/ctldatabase.h"
#include "models/stepfunctionmodels.h"
#include <cmath>

namespace CTL {

static void grindBall(VoxelVolume<float>& volume, float radius);

/*!
 * \brief Constructs a SpectralVolumeData representing the attenuation coefficients \a muValues.
 *
 * This is a convenience constructor, mainly intended to allow for implicit casts of
 * VoxelVolume<float> to SpectralVolumeData.
 *
 * No spectral information is available in the resulting object. It is strongly encouraged to use
 * VoxelVolume<float> directly when managing non-spectral attenuation information.
 */
SpectralVolumeData::SpectralVolumeData(VoxelVolume<float> muValues)
    : VoxelVolume<float> (std::move(muValues))
    , _absorptionModel(new ConstantModel())
    , _isMu(true)
    , _refEnergy(42.0f)
    , _refMassAttenuationCoeff(1.0f)

{
}

/*!
 * \brief Constructs a SpectralVolumeData representing the attenuation coefficients \a muValues
 * (in 1/mm) with respect to the given \a referenceEnergy (in keV) for a material described by
 * \a absorptionModel.
 */
SpectralVolumeData::SpectralVolumeData(VoxelVolume<float> muValues,
                                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                       float referenceEnergy,
                                       const QString& materialName)
    : VoxelVolume<float> (std::move(muValues))
{
    if(referenceEnergy < 0.0f)
        throw std::runtime_error("SpectralVolumeData::SpectralVolumeData: Cannot create volume: No "
                                 "negative reference energies allowed.");
    if(!absorptionModel)
        throw std::runtime_error("SpectralVolumeData::SpectralVolumeData: Invalid absorption model (nullptr)!");

    _absorptionModel = std::move(absorptionModel);
    _hasNonDefaultAbsModel = true;
    _materialName = materialName.isEmpty() ? _absorptionModel->name() : materialName;
    _isMu = true;
    _refEnergy = referenceEnergy;
    _refMassAttenuationCoeff = _absorptionModel->valueAt(referenceEnergy);
}

/*!
 * \brief Constructs a SpectralVolumeData representing the density values \a materialDensity
 * (in g/cm^-3) for a material described by \a absorptionModel.
 *
 *
 */
SpectralVolumeData::SpectralVolumeData(VoxelVolume<float> materialDensity,
                                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                       const QString& materialName)
    : VoxelVolume<float> (std::move(materialDensity))
{
    if(!absorptionModel)
        throw std::runtime_error("SpectralVolumeData::SpectralVolumeData: Invalid absorption model (nullptr)!");

    _absorptionModel = std::move(absorptionModel);
    _hasNonDefaultAbsModel = true;
    _materialName = materialName.isEmpty() ? _absorptionModel->name() : materialName;
}

/*!
 * Returns the absorption model of this instance.
 *
 * The absorption model represents a single material (either elemental or composite) and contains
 * the spectral dependency of its mass attenuation coefficients (in cm^2/g).
 */
std::shared_ptr<AbstractIntegrableDataModel> SpectralVolumeData::absorptionModel() const
{
    return _absorptionModel;
}

/*!
 * Returns the mass attenuation coefficient of the material described by this instance averaged over
 * the energy bin [\a centerEnergy - \a binWidth / 2, \a centerEnergy + \a binWidth / 2].
 *
 * Same as absorptionModel()->meanValue(centerEnergy, binWidth).
 */
float SpectralVolumeData::meanMassAttenuationCoeff(float centerEnergy, float binWidth) const
{
    if(qFuzzyIsNull(binWidth))
    {
        qWarning("SpectralVolumeData::averageMassAttenuationFactor: Interval width is zero!"
                 "Delegating call to SpectralVolumeData::massAttenuationFactor.");
        // return massAttenuationCoeff(centerEnergy); <-- not required, covered by meanValue()
    }

    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::averageMassAttenuationFactor: No absorption model set!");

    return _absorptionModel->meanValue(centerEnergy, binWidth);
}

/*!
 * Returns the density representation of this instance.
 *
 * Note that this creates a copy of the data. In case this instance does already contain density
 * data (check this via isDensityVolume()) it is discouraged to call this method because it would
 * only create an unnecessary copy.
 */
SpectralVolumeData SpectralVolumeData::densityVolume() const
{
    SpectralVolumeData ret(*this);

    if(isMuVolume())
        ret.transformToDensity();

    return ret;
}

/*!
 * Returns true if this instance has full spectral information. This means the following conditions
 * are fulfilled:
 *
 * - a data model describing the spectral dependency of the mass-attenuation coefficient for the
 *   material represented by this volume has been set
 * - one of the following conditions is fulfilled:
 *      * numerical values in individual voxels represent material density (in g/cm^-3), or
 *      * numerical values in individual voxels represent attenuation coefficiens (in 1/mm) and the
 *        corresponding reference energy is specified.
 */
bool SpectralVolumeData::hasSpectralInformation() const
{
    if(!_hasNonDefaultAbsModel) // requires attenuation model
        return false;

    if(isMuVolume()) // also requires reference energy and att. coeff information
        return (_refEnergy >= 0.0f) && (_refMassAttenuationCoeff >= 0.0f);

    return true;
}

bool SpectralVolumeData::isDensityVolume() const
{
    return !_isMu;
}

bool SpectralVolumeData::isMuVolume() const
{
    return _isMu;
}

float SpectralVolumeData::massAttenuationCoeff(float atEnergy) const
{
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::massAttenuationFactor: No absorption model set!");

    return _absorptionModel->valueAt(atEnergy);
}

const QString& SpectralVolumeData::materialName() const
{
    return _materialName;
}

SpectralVolumeData SpectralVolumeData::muVolume(float referenceEnergy) const
{
    SpectralVolumeData ret(*this);

    if(isMuVolume()) // change reference energy
        ret.changeReferenceEnergy(referenceEnergy);
    else // transform to mu values from density
        ret.transformToAttenuationCoeff(referenceEnergy);

    return ret;
}

SpectralVolumeData SpectralVolumeData::muVolume(float centerEnergy, float binWidth) const
{
    SpectralVolumeData ret(*this);

    const auto meanMassAttCoeff = meanMassAttenuationCoeff(centerEnergy, binWidth);

    if(isMuVolume()) // change reference mass attenuation coeff
        ret.changeReferenceMassAttCoeff(meanMassAttCoeff, centerEnergy);
    else // transform to mu values from density
        ret.transformToAttenuationCoeff(meanMassAttCoeff, centerEnergy);

    return ret;
}

float SpectralVolumeData::referenceEnergy() const
{
    if(!isMuVolume())
        qWarning() << "Reference information meaningless: Volume does not contain attenuation"
                      "coefficients";
    return _refEnergy;
}

float SpectralVolumeData::referenceMassAttenuationCoeff() const
{
    if(!isMuVolume())
        qWarning() << "Reference information meaningless: Volume does not contain attenuation"
                      "coefficients";
    return _refMassAttenuationCoeff;
}

SpectralVolumeData SpectralVolumeData::createBall(float radius, float voxelSize, float muValue)
{
    const auto nbVox = static_cast<uint>(std::ceil(2.0f * radius / voxelSize));

    const SpectralVolumeData::Dimensions volDim{ nbVox, nbVox, nbVox };
    const SpectralVolumeData::VoxelSize voxSize{ voxelSize, voxelSize, voxelSize };
    SpectralVolumeData ret{ { volDim, voxSize } };
    ret.fill(muValue);

    grindBall(ret, radius);

    return ret;
}

SpectralVolumeData
SpectralVolumeData::createBall(float radius,
                               float voxelSize,
                               float density,
                               std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    const auto nbVox = static_cast<uint>(std::ceil(2.0f * radius / voxelSize));

    const SpectralVolumeData::Dimensions volDim{ nbVox, nbVox, nbVox };
    const SpectralVolumeData::VoxelSize voxSize{ voxelSize, voxelSize, voxelSize };
    SpectralVolumeData ret{ { volDim, voxSize }, std::move(absorptionModel) };
    ret.fill(density);

    grindBall(ret, radius);

    return ret;
}

void SpectralVolumeData::replaceAbsorptionModel(AbstractIntegrableDataModel* absorptionModel)
{
    if(!hasSpectralInformation())
        throw std::runtime_error("SpectralVolumeData::replaceAbsorptionModel: Volume does not "
                                 "contain spectral information!");

    if(!absorptionModel)
        throw std::runtime_error("SpectralVolumeData::setAbsorptionModel: Invalid absorption model (nullptr)!");

    _absorptionModel.reset(absorptionModel);
}

void SpectralVolumeData::replaceAbsorptionModel(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    if(!hasSpectralInformation())
        throw std::runtime_error("SpectralVolumeData::replaceAbsorptionModel: Volume does not "
                                 "contain spectral information!");

    if(!absorptionModel)
        throw std::runtime_error("SpectralVolumeData::setAbsorptionModel: Invalid absorption model (nullptr)!");

    _absorptionModel = std::move(absorptionModel);
}

void SpectralVolumeData::setDensity(VoxelVolume<float> density)
{
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::setDensity: Cannot set density values: No "
                                 "absorption model set!");

    *this = std::move(density);
    _isMu = false;
}

void SpectralVolumeData::setMaterialName(const QString& name)
{
    _materialName = name;
}

SpectralVolumeData SpectralVolumeData::fromMuVolume(VoxelVolume<float> muValues,
                                                    std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                                    float referenceEnergy)
{
    if(referenceEnergy < 0.0f)
        throw std::runtime_error("SpectralVolumeData::fromMuVolume: Cannot create volume: No "
                                 "negative reference energies allowed.");

    constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm

    // transform to densities (g/cm^3)
    muValues /= (cm2mm * absorptionModel->valueAt(referenceEnergy));

    return SpectralVolumeData(std::move(muValues), absorptionModel, absorptionModel->name());
}

SpectralVolumeData SpectralVolumeData::fromHUVolume(VoxelVolume<float> HUValues,
                                                    std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                                    float referenceEnergy)
{
    if(referenceEnergy < 0.0f)
        throw std::runtime_error("SpectralVolumeData::fromMuVolume: Cannot create volume: No "
                                 "negative reference energies allowed.");

    constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm
    constexpr auto HUscaleFactor = 1000.0f;

    // transform to attenuation values
    auto muWater = database::attenuationModel(database::Composite::Water)->valueAt(referenceEnergy);
    HUValues = (HUValues*(muWater/HUscaleFactor)) + muWater;

    // transform to densities (g/cm^3)
    HUValues /= (cm2mm * absorptionModel->valueAt(referenceEnergy));

    return SpectralVolumeData(std::move(HUValues), absorptionModel, absorptionModel->name());
}

void SpectralVolumeData::changeReferenceEnergy(float newReferenceEnergy)
{
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::changeReferenceEnergy: Cannot change reference"
                                 " energy: No absorption model set!");

    const auto newRefMassAttuationCoeff = _absorptionModel->valueAt(newReferenceEnergy);
    (*this) *= newRefMassAttuationCoeff / _refMassAttenuationCoeff;

    _refEnergy = newReferenceEnergy;
    _refMassAttenuationCoeff = newRefMassAttuationCoeff;
}

void SpectralVolumeData::changeReferenceMassAttCoeff(float newReferenceMassAttCoeff, float correspondingRefEnergy)
{
    (*this) *= newReferenceMassAttCoeff / _refMassAttenuationCoeff;

    _refEnergy = correspondingRefEnergy;
    _refMassAttenuationCoeff = newReferenceMassAttCoeff;
}

void SpectralVolumeData::transformToAttenuationCoeff(float referenceEnergy)
{
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::transformToAttenuationCoeff: Cannot transform"
                                 " to attenuation coefficients: No absorption model set!");

    constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm
    const auto refMassAttenuationCoeff = _absorptionModel->valueAt(referenceEnergy);
    (*this) *= refMassAttenuationCoeff * cm2mm;

    _isMu = true;
    _refEnergy = referenceEnergy;
    _refMassAttenuationCoeff = refMassAttenuationCoeff;
}

void SpectralVolumeData::transformToAttenuationCoeff(float referenceMassAttCoeff, float correspondingRefEnergy)
{
    constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm
    (*this) *= referenceMassAttCoeff * cm2mm;

    _isMu = true;
    _refEnergy = correspondingRefEnergy;
    _refMassAttenuationCoeff = referenceMassAttCoeff;
}

void SpectralVolumeData::transformToDensity()
{
    if(!hasSpectralInformation())
        throw std::runtime_error("SpectralVolumeData::transformToDensity: Cannot transform"
                                 " to density values: Insufficient spectral information!");

    constexpr auto cm2mm = 0.1f; // 1/cm -> 1/mm
    (*this) /= _refMassAttenuationCoeff * cm2mm;

    _isMu = false;
}

void grindBall(VoxelVolume<float>& volume, float radius)
{
    const auto nbVox = volume.dimensions().x;
    const auto center = float(nbVox - 1) / 2.0f;

    auto dist2Center = [center](float x, float y, float z)
    {
        const auto dx = x - center;
        const auto dy = y - center;
        const auto dz = z - center;
        return dx * dx + dy * dy + dz * dz;
    };

    const auto voxSize = volume.voxelSize().x;
    const auto rSquaredInVoxel = (radius / voxSize) * (radius / voxSize);

    // erase exterior space
    for(auto x = 0u; x < nbVox; ++x)
        for(auto y = 0u; y < nbVox; ++y)
            for(auto z = 0u; z < nbVox; ++z)
                if(dist2Center(x, y, z) > rSquaredInVoxel)
                    volume(x, y, z) = 0.0f;
}

} // namespace CTL
