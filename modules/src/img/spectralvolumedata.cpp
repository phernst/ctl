#include "spectralvolumedata.h"
#include "io/ctldatabase.h"
#include "models/stepfunctionmodels.h"
#include <cmath>

namespace CTL {

//static void grindBall(VoxelVolume<float>& volume, float radius);

/*!
 * \brief Constructs a SpectralVolumeData representing the attenuation coefficients \a muValues
 * (in 1/mm).
 *
 * This is a convenience constructor, mainly intended to allow for implicit casts of
 * VoxelVolume<float> to SpectralVolumeData.
 *
 * Note that it is not meaningful to use this constructor with input data in Hounsfield units (HU).
 *
 * No spectral information is available in the resulting object. It is strongly encouraged to use
 * VoxelVolume<float> directly when managing non-spectral attenuation information.
 */
SpectralVolumeData::SpectralVolumeData(VoxelVolume<float> muValues)
    : VoxelVolume<float> (std::move(muValues))
    , _absorptionModel(new ConstantModel)
    , _isMu(true)
    , _refEnergy(42.0f)
    , _refMassAttenuationCoeff(1.0f)
{
}

/*!
 * \brief Constructs a SpectralVolumeData representing the attenuation coefficients \a muValues
 * (in 1/mm) with respect to the given \a referenceEnergy (in keV) for a material described by
 * \a absorptionModel.
 *
 * Note that this can only be used when \a muValues holds values in 1/mm. To create from Hounsfield
 * units (HU), use fromHUVolume().
 *
 * Keeps internally stored data in attenuation domain. The factory method fromMuVolume() has similar
 * function, but transforms data to density domain.
 */
SpectralVolumeData::SpectralVolumeData(VoxelVolume<float> muValues,
                                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                       float referenceEnergy,
                                       const QString& materialName)
    : VoxelVolume<float>(std::move(muValues))
    , _absorptionModel(std::move(absorptionModel))
    , _materialName(materialName.isEmpty() ? _absorptionModel->name() : materialName)
    , _refEnergy(referenceEnergy)
{
    if(_refEnergy < 0.0f)
        throw std::runtime_error("SpectralVolumeData::SpectralVolumeData: Cannot create volume: No "
                                 "negative reference energies allowed.");
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::SpectralVolumeData: Invalid absorption model "
                                 "(nullptr)!");

    _hasNonDefaultAbsModel = true;
    _isMu = true;
    _refMassAttenuationCoeff = _absorptionModel->valueAt(_refEnergy);
}

/*!
 * \brief Constructs a SpectralVolumeData representing the density values \a materialDensity
 * (in g/cm^3) for a material described by \a absorptionModel.
 *
 *
 */
SpectralVolumeData::SpectralVolumeData(VoxelVolume<float> materialDensity,
                                       std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                       const QString& materialName)
    : VoxelVolume<float>(std::move(materialDensity))
    , _absorptionModel(std::move(absorptionModel))
    , _materialName(materialName.isEmpty() ? _absorptionModel->name() : materialName)
{
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::SpectralVolumeData: Invalid absorption model "
                                 "(nullptr)!");

    _hasNonDefaultAbsModel = true;
}

SpectralVolumeData* SpectralVolumeData::clone() const
{
    return new SpectralVolumeData(*this);
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
std::unique_ptr<SpectralVolumeData> SpectralVolumeData::densityVolume() const
{
    auto ret = std::unique_ptr<SpectralVolumeData>(this->clone());

    if(isMuVolume())
        ret->transformToDensity();

    return ret;
}

/*!
 * Returns true if this instance has full spectral information. This means the following conditions
 * are fulfilled:
 *
 * - a data model describing the spectral dependency of the mass-attenuation coefficient for the
 *   material represented by this volume has been set
 * - one of the following conditions is fulfilled:
 *      * numerical values in individual voxels represent material density (in g/cm^3), or
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

/*!
 * Returns true if the data stored by this instance are density values (in g/cm^3).
 *
 * \code
 * auto volume = SpectralVolumeData(VoxelVolume<float>::cube(100, 1.0f, 0.02f),
 *                                  database::attenuationModel(database::Composite::Water),
 *                                  65.0f);
 *
 * qInfo() << volume.isDensityVolume(); // output: false  <-- constructor keeps attenuation domain
 *
 * // This time, we use the factory method.
 * auto volume2 = SpectralVolumeData::fromMuVolume(VoxelVolume<float>::cube(100, 1.0f, 0.02f),
 *                                                 database::attenuationModel(database::Composite::Water),
 *                                                 65.0f);
 *
 * qInfo() << volume2.isDensityVolume(); // output: true  <-- fromMuVolume() transforms to density
 * \endcode
 */
bool SpectralVolumeData::isDensityVolume() const { return !_isMu; }

/*!
 * Returns true if the data stored by this instance are attenuation values (in 1/mm).
 *
 * \code
 * auto volume = SpectralVolumeData(VoxelVolume<float>::cube(100, 1.0f, 0.02f),
 *                                  database::attenuationModel(database::Composite::Water),
 *                                  65.0f);
 *
 * qInfo() << volume.isDensityVolume(); // output: false  <-- constructor keeps attenuation domain
 *
 * // This time, we use the factory method.
 * auto volume2 = SpectralVolumeData::fromMuVolume(VoxelVolume<float>::cube(100, 1.0f, 0.02f),
 *                                                 database::attenuationModel(database::Composite::Water),
 *                                                 65.0f);
 *
 * qInfo() << volume2.isDensityVolume(); // output: true  <-- fromMuVolume() transforms to density
 * \endcode
 */
bool SpectralVolumeData::isMuVolume() const { return _isMu; }

/*!
 * Returns the mass attenuation coefficient (in cm^2/g) of the material described by this instance
 * for energy \a atEnergy (in keV).
 *
 * Same as absorptionModel()->valueAt(atEnergy).
 */
float SpectralVolumeData::massAttenuationCoeff(float atEnergy) const
{
    if(!_absorptionModel)
        throw std::runtime_error("SpectralVolumeData::massAttenuationFactor: No absorption model set!");

    return _absorptionModel->valueAt(atEnergy);
}

/*!
 * Returns the name of the material described by this instance.
 */
const QString& SpectralVolumeData::materialName() const
{
    return _materialName;
}

///
/// Returns the attenuation coefficient (with respect to the reference energy \a referenceEnergy)
/// representation of this instance.
///
/// Note that this creates a copy of the data. The density values are transformed to attenuation
/// coefficients with respect to the given reference energy \a referenceEnergy. In case this
/// instance does already contain attenuation coefficient data, the values are re-referenced to
/// \a referenceEnergy.
///
/// If this instance contains attenuation coefficient data and
/// \a referenceEnergy == referenceEnergy(), it is discouraged to call this method because it would
/// only create an unnecessary copy.
///
/// \code
/// // Starting point: voxel data with attenuation coefficients in 1/mm
/// // here: make a cube phantom (100^3 voxels, size 1 mm^3) filled with value 0.02/mm (water)
/// VoxelVolume<float> muValues(100, 100, 100, 1.0f, 1.0f, 1.0f);
/// muValues.fill(0.02f); // 0.02 / mm is approx. attenuation of water for 60 keV
///
/// // Now: create spectral data by complementing \c muValues with the attenuation model of water
/// auto volume = SpectralVolumeData(muValues,              /* std::move(muValues), if no longer required. */
///                                  database::attenuationModel(database::Composite::Water),
///                                  60.0f);
///
/// qInfo() << volume.max();                                // output: 0.02 (our input, still in attenuation domain)
/// qInfo() << volume.referenceEnergy();                    // output: 60
/// qInfo() << volume.referenceMassAttenuationCoeff();      // output: 0.2059
///
/// // auto otherVolume = volume.muVolume(60.0f);           // meaningless copy, result identical to 'volume'
/// auto otherVolume = volume.muVolume(45.0f);              // changes reference energy from 60 keV to 45 keV
///
/// qInfo() << otherVolume.max();                           // output: 0.0240505 (rescaled to new reference energy)
/// qInfo() << otherVolume.referenceEnergy();               // output: 45
/// qInfo() << otherVolume.referenceMassAttenuationCoeff(); // output: 0.2476
/// \endcode
///

std::unique_ptr<SpectralVolumeData> SpectralVolumeData::muVolume(float referenceEnergy) const
{
    auto ret = std::unique_ptr<SpectralVolumeData>(this->clone());

    if(isMuVolume()) // change reference energy
        ret->changeReferenceEnergy(referenceEnergy);
    else // transform to mu values from density
        ret->transformToAttenuationCoeff(referenceEnergy);

    return ret;
}

/*!
 * Returns the attenuation coefficient with respect to an average mass attenuation coefficient in
 * the energy interval [\a centerEnergy - \a binWidth / 2, \a centerEnergy + \a binWidth / 2].
 *
 * Note that this creates a copy of the data.
 *
 * \sa muVolume(float), meanMassAttenuationCoeff().
 */
std::unique_ptr<SpectralVolumeData> SpectralVolumeData::muVolume(float centerEnergy, float binWidth) const
{
    auto ret = std::unique_ptr<SpectralVolumeData>(this->clone());

    const auto meanMassAttCoeff = meanMassAttenuationCoeff(centerEnergy, binWidth);

    if(isMuVolume()) // change reference mass attenuation coeff
        ret->changeReferenceMassAttCoeff(meanMassAttCoeff, centerEnergy);
    else // transform to mu values from density
        ret->transformToAttenuationCoeff(meanMassAttCoeff, centerEnergy);

    return ret;
}

/*!
 * Returns the reference energy corresponding to the attenuation values managed by this instance.
 * Does not contain meaningful information in case this instance manages density values.
 */
float SpectralVolumeData::referenceEnergy() const
{
    if(!isMuVolume())
        qWarning() << "Reference information meaningless: Volume does not contain attenuation"
                      "coefficients";
    return _refEnergy;
}

/*!
 * Returns the reference mass attenuation coefficient corresponding to the attenuation values
 * managed by this instance.
 * Does not contain meaningful information in case this instance manages density values.
 */
float SpectralVolumeData::referenceMassAttenuationCoeff() const
{
    if(!isMuVolume())
        qWarning() << "Reference information meaningless: Volume does not contain attenuation"
                      "coefficients";
    return _refMassAttenuationCoeff;
}

/*!
 * Creates a SpectralVolumeData object that represents a voxelized ball with radius \a radius,
 * isometric voxel size \a voxelSize (both in mm) and filled (homogeneuosly) with density value
 * \a density (in g/cm^3). The material properties (i.e. spectrally-dependent mass attenuation
 * coefficients) are specified by \a absorptionModel. The voxels surrounding the ball are filled
 * with density 0.0 g/cm^3.
 */
SpectralVolumeData
SpectralVolumeData::ball(float radius, float voxelSize, float density,
                         std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    return { VoxelVolume<float>::ball(radius, voxelSize, density), std::move(absorptionModel) };
}

/*!
 * Constructs a cubic SpectralVolumeData with \a nbVoxel x \a nbVoxel x \a nbVoxel voxels (voxel
 * dimension: \a voxelSize x \a voxelSize x \a voxelSize), filled (homogeneuosly) with density value
 * \a density (in g/cm^3). The material properties (i.e. spectrally-dependent mass attenuation
 * coefficients) are specified by \a absorptionModel.
 */
SpectralVolumeData
SpectralVolumeData::cube(uint nbVoxel, float voxelSize, float density,
                         std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    return { VoxelVolume<float>::cube(nbVoxel, voxelSize, density), std::move(absorptionModel) };
}

/*!
 * Creates a SpectralVolumeData object that represents a voxelized cylinder with radius \a radius
 * and height \a height (both in mm) that is aligned with the *x*-axis. It has isometric voxel
 * size \a voxelSize (in mm) and is filled (homogeneuosly) with density value \a density
 * (in g/cm^3). The material properties (i.e. spectrally-dependent mass attenuation coefficients)
 * are specified by \a absorptionModel. The voxels surrounding the cylinder are filled with density
 * 0.0 g/cm^3.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * *y*- and *z*-dimension and \f$ \left\lceil height/voxelSize\right\rceil \f$ in *x*-direction.
 */
SpectralVolumeData
SpectralVolumeData::cylinderX(float radius, float height, float voxelSize, float density,
                              std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    return { VoxelVolume<float>::cylinderX(radius, height, voxelSize, density),
             std::move(absorptionModel) };
}

/*!
 * Creates a SpectralVolumeData object that represents a voxelized cylinder with radius \a radius
 * and height \a height (both in mm) that is aligned with the *y*-axis. It has isometric voxel
 * size \a voxelSize (in mm) and is filled (homogeneuosly) with density value \a density
 * (in g/cm^3). The material properties (i.e. spectrally-dependent mass attenuation coefficients)
 * are specified by \a absorptionModel. The voxels surrounding the cylinder are filled with density
 * 0.0 g/cm^3.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * *x*- and *z*-dimension and \f$ \left\lceil height/voxelSize\right\rceil \f$ in *y*-direction.
 */
SpectralVolumeData
SpectralVolumeData::cylinderY(float radius, float height, float voxelSize, float density,
                              std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    return { VoxelVolume<float>::cylinderY(radius, height, voxelSize, density),
             std::move(absorptionModel) };
}

/*!
 * Creates a SpectralVolumeData object that represents a voxelized cylinder with radius \a radius
 * and height \a height (both in mm) that is aligned with the *z*-axis. It has isometric voxel
 * size \a voxelSize (in mm) and is filled (homogeneuosly) with density value \a density
 * (in g/cm^3). The material properties (i.e. spectrally-dependent mass attenuation coefficients)
 * are specified by \a absorptionModel. The voxels surrounding the cylinder are filled with density
 * 0.0 g/cm^3.
 *
 * The resulting volume will have \f$ \left\lceil 2\cdot radius/voxelSize\right\rceil \f$ voxels in
 * *x*- and *y*-dimension and \f$ \left\lceil height/voxelSize\right\rceil \f$ in *z*-direction.
 */
SpectralVolumeData
SpectralVolumeData::cylinderZ(float radius, float height, float voxelSize, float density,
                              std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    return { VoxelVolume<float>::cylinderZ(radius, height, voxelSize, density),
             std::move(absorptionModel) };
}

/*!
 * Replaces the absorption model in this instance by \a absorptionModel (must not be nullptr).
 *
 * Note that this is only allowed for objects that already have spectral information available and
 * throws an std::runtime_error otherwise.
 */
void SpectralVolumeData::replaceAbsorptionModel(AbstractIntegrableDataModel* absorptionModel)
{
    if(!hasSpectralInformation())
        throw std::runtime_error("SpectralVolumeData::replaceAbsorptionModel: Volume does not "
                                 "contain spectral information!");

    if(!absorptionModel)
        throw std::runtime_error("SpectralVolumeData::setAbsorptionModel: Invalid absorption model (nullptr)!");

    _absorptionModel.reset(absorptionModel);
}

/*!
 * Replaces the absorption model in this instance by \a absorptionModel (must not be nullptr).
 *
 * Note that this is only allowed for objects that already have spectral information available and
 * throws an std::runtime_error otherwise.
 */
void SpectralVolumeData::replaceAbsorptionModel(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel)
{
    if(!hasSpectralInformation())
        throw std::runtime_error("SpectralVolumeData::replaceAbsorptionModel: Volume does not "
                                 "contain spectral information!");

    if(!absorptionModel)
        throw std::runtime_error("SpectralVolumeData::setAbsorptionModel: Invalid absorption model (nullptr)!");

    _absorptionModel = std::move(absorptionModel);
}

/*!
 * Replaces the voxel data in this instance by the density values given by \a density (in g/cm^3).
 *
 * Note that this is only allowed for objects that have a non-defaulted absorbtion model set and
 * throws an std::runtime_error otherwise.
 */
void SpectralVolumeData::setDensity(VoxelVolume<float> density)
{
    if(!_hasNonDefaultAbsModel)
        throw std::runtime_error("SpectralVolumeData::setDensity: Cannot set density values: No "
                                 "absorption model set!");

    *this = std::move(density);
    _isMu = false;
}

/*!
 * Sets the name of the material described by this instance to \a name.
 */
void SpectralVolumeData::setMaterialName(const QString& name)
{
    _materialName = name;
}

///
/// Creates a SpectralVolumeData object from the attenuation values given by \a muValues (in 1 / mm)
/// corresponding to the reference energy \a referenceEnergy of the material specified by its
/// spectrally-dependent mass attenuation coefficients in \a absorptionModel.
///
/// Generates the density representation of the data. To prevent transformation into density domain
/// (e.g. if follow-up processing needs to be done in attenuation domain anyway), use the constructor
/// with the same input instead.
///
///  \code
///  // Starting point: voxel data with attenuation coefficients in 1/mm
///  // here: make a cube phantom (100^3 voxels, size 1 mm^3) representing cortical bone
///  auto volumeMu = VoxelVolume<float>(100, 100, 100, 1.0f, 1.0f, 1.0f);
///  volumeMu.fill(0.056f); // fill with value 0.056/mm <-- approx. attenuation coeff. of bone @ 65 keV
///
///  // create spectral volume from mu (i.e. att. coeff.) data (cortical bone data, reference energy of 65 keV)
///  auto volume = SpectralVolumeData::fromMuVolume(volumeMu,  /* std::move(volumeMu), if no longer required. */
///                                                 database::attenuationModel(database::Composite::Bone_Cortical),
///                                                 65.0f);
///
///  qInfo() << volume.max(); // output: 1.91896 (g/cm^3) <-- density representation
/// \endcode
///
/// \sa muVolume()
///
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

///
/// Creates a SpectralVolumeData object from the attenuation values given by \a HUValues (in
/// Hounsfield units) representing the material specified by its spectrally-dependent mass
/// attenuation coefficients in \a absorptionModel. For meaningful results, you need to also specify
/// the reference energy, to which the Hounsfield units correspond, by \a referenceEnergy (in keV).
///
/// Generates the density representation of the data.
///
/// \code
/// // Starting point: voxel data in Hounsfield units (eg. loaded patient data)
/// // here: make a cube phantom (100^3 voxels, size 1 mm^3) filled with value 100 HU
/// auto volumeHU = VoxelVolume<float>(100, 100, 100, 1.0f, 1.0f, 1.0f);
/// volumeHU.fill(100.0f); // fill with value 100 HU
///
/// // Now: create spectral volume from HU data (soft tissue data, reference energy of 62 keV)
/// auto volume = SpectralVolumeData::fromHUVolume(volumeHU,   /* std::move(volumeHU), if no longer required. */
///                                                database::attenuationModel(database::Composite::Tissue_Soft),
///                                                62.0f);
///
/// qInfo() << volume.max(); // output: 1.10614 (g/cm^3)  <-- density representation
/// \endcode
///
SpectralVolumeData SpectralVolumeData::fromHUVolume(VoxelVolume<float> HUValues,
                                                    std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                                    float referenceEnergy)
{
    if(referenceEnergy < 0.0f)
        throw std::runtime_error("SpectralVolumeData::fromMuVolume: Cannot create volume: No "
                                 "negative reference energies allowed.");

    constexpr auto HUscaleFactor = 1000.0f;

    // transform to attenuation values
    auto muWater = database::attenuationModel(database::Composite::Water)->valueAt(referenceEnergy);
    HUValues = (HUValues*(muWater/HUscaleFactor)) + muWater;

    // transform to densities (g/cm^3)
    HUValues /= absorptionModel->valueAt(referenceEnergy);

    return SpectralVolumeData(std::move(HUValues), absorptionModel, absorptionModel->name());
}

// ###############
// private methods
// ###############

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

//void grindBall(VoxelVolume<float>& volume, float radius)
//{
//    const auto nbVox = volume.dimensions().x;
//    const auto center = float(nbVox - 1) / 2.0f;

//    auto dist2Center = [center](float x, float y, float z)
//    {
//        const auto dx = x - center;
//        const auto dy = y - center;
//        const auto dz = z - center;
//        return dx * dx + dy * dy + dz * dz;
//    };

//    const auto voxSize = volume.voxelSize().x;
//    const auto rSquaredInVoxel = (radius / voxSize) * (radius / voxSize);

//    // erase exterior space
//    for(auto x = 0u; x < nbVox; ++x)
//        for(auto y = 0u; y < nbVox; ++y)
//            for(auto z = 0u; z < nbVox; ++z)
//                if(dist2Center(x, y, z) > rSquaredInVoxel)
//                    volume(x, y, z) = 0.0f;
//}

} // namespace CTL
