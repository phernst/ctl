#include "modelbasedvolumedecomposer.h"
#include "models/stepfunctionmodels.h"

namespace CTL {

ModelBasedVolumeDecomposer::ModelBasedVolumeDecomposer()
{

}

CompositeVolume ModelBasedVolumeDecomposer::decompose(const VoxelVolume<float> &volume, float referenceEnergy) const
{
    CompositeVolume ret;

    for(uint mat = 0; mat < nbMaterials(); ++mat)
    {
        SpectralVolumeData matVol = SpectralVolumeData::fromMuVolume(volume, _absorptionModels[mat], referenceEnergy);

        auto datPtr = matVol.rawData();
        for(size_t vox = 0; vox < size_t(matVol.totalVoxelCount()); ++vox)
            datPtr[vox] = datPtr[vox] * _segmentationModels[mat]->valueAt(datPtr[vox]);

        ret.addMaterialVolume(std::move(matVol));
    }

    return ret; }

void ModelBasedVolumeDecomposer::addMaterial(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                                             std::shared_ptr<AbstractDataModel> segmentationModel)
{
    _absorptionModels.push_back(absorptionModel);
    _segmentationModels.push_back(segmentationModel);
}

uint ModelBasedVolumeDecomposer::nbMaterials() const
{
    return static_cast<uint>(_segmentationModels.size());
}

TwoMaterialThresholdVolumeDecomposer::TwoMaterialThresholdVolumeDecomposer(std::shared_ptr<AbstractIntegrableDataModel> absorptionModelMaterial1,
                                                                           std::shared_ptr<AbstractIntegrableDataModel> absorptionModelMaterial2,
                                                                           float thresholdDensity1)
    : _absMaterial1(absorptionModelMaterial1)
    , _absMaterial2(absorptionModelMaterial2)
    , _threshold(thresholdDensity1)
{
}

CompositeVolume TwoMaterialThresholdVolumeDecomposer::decompose(const VoxelVolume<float> &volume, float referenceEnergy) const
{
    ModelBasedVolumeDecomposer decomposer;
    float scale = _absMaterial1->valueAt(referenceEnergy) / _absMaterial2->valueAt(referenceEnergy);
    decomposer.addMaterial(_absMaterial1, std::make_shared<StepFunctionModel>(_threshold, 1.0f, false));
    decomposer.addMaterial(_absMaterial2, std::make_shared<StepFunctionModel>(_threshold * scale, 1.0f, true));

    return decomposer.decompose(volume, referenceEnergy);
}

} // namespace CTL
