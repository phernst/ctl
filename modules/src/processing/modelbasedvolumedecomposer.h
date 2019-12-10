#ifndef MODELBASEDVOLUMEDECOMPOSER_H
#define MODELBASEDVOLUMEDECOMPOSER_H

#include "abstractvolumedecomposer.h"

namespace CTL {

class ModelBasedVolumeDecomposer : public AbstractVolumeDecomposer
{
    public: CompositeVolume decompose(const VoxelVolume<float>& volume,
                                      float referenceEnergy = 50.0f) const override;

public:
    void addMaterial(std::shared_ptr<AbstractIntegrableDataModel> absorptionModel,
                     std::shared_ptr<AbstractDataModel> segmentationModel);
    uint nbMaterials() const;

protected:
    std::vector<std::shared_ptr<AbstractIntegrableDataModel>> _absorptionModels;
    std::vector<std::shared_ptr<AbstractDataModel>> _segmentationModels;
};


class TwoMaterialThresholdVolumeDecomposer : public AbstractVolumeDecomposer
{
    public: CompositeVolume decompose(const VoxelVolume<float>& volume,
                                      float referenceEnergy = 50.0f) const override;

public:
    TwoMaterialThresholdVolumeDecomposer(
        std::shared_ptr<AbstractIntegrableDataModel> absorptionModelMaterial1,
        std::shared_ptr<AbstractIntegrableDataModel> absorptionModelMaterial2,
        float thresholdDensity1);

private:
    std::shared_ptr<AbstractIntegrableDataModel> _absMaterial1;
    std::shared_ptr<AbstractIntegrableDataModel> _absMaterial2;
    float _threshold;
};

} // namespace CTL

#endif // MODELBASEDVOLUMEDECOMPOSER_H
