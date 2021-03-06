#include "basisfunctionvolume.h"

namespace CTL {

void BasisFunctionVolume::updateVolume()
{
    this->fill(0.0f);

    const auto discreteTime = time2Sample(this->time());

    if(discreteTime >= _model->basisFcts.front().size())
        return;

    auto updatedVol = std::inner_product(
        // iterate over all coefficients/basis functions
        _model->coeffVolumes.cbegin(), _model->coeffVolumes.cend(), _model->basisFcts.cbegin(),
        // init volume (zero initialized, see above)
        VoxelVolume<float>(*this),
        // the "+" operation
        std::plus<VoxelVolume<float>>(),
        // the "*" operation
        [discreteTime](const VoxelVolume<float>& v, const std::vector<float>& f) {
            return v * f[discreteTime];
        }
    );

    this->setData(std::move(updatedVol.data()));
}

SpectralVolumeData* BasisFunctionVolume::clone() const
{
    return new BasisFunctionVolume(*this);
}

BasisFunctionVolume::BasisFunctionVolume(BasisFunctionVolume::CoeffVolumes coeffVolumes,
                                         SampledFunctions basisFunctions)
    : AbstractDynamicVolumeData(
        VoxelVolume<float>(coeffVolumes.at(0).nbVoxels(), coeffVolumes.at(0).voxelSize()))
    , _model(std::make_shared<const ModelParameters>(
          ModelParameters{ std::move(coeffVolumes), std::move(basisFunctions) }))
{
    // check for consistent sizes
    if(_model->basisFcts.size() != _model->coeffVolumes.size())
    {
        throw std::runtime_error(errMsgDifferentNumberOfCoeffsAndBasisFcts());
    }

    if(std::any_of(std::next(_model->coeffVolumes.cbegin()), _model->coeffVolumes.cend(),
                   [this](const VoxelVolume& c) {
                       return c.nbVoxels() != this->nbVoxels()
                           || c.voxelSize() != this->voxelSize();
                   }))
    {
        throw std::runtime_error(errMsgInconsistentVolumes());
    }

    if(std::any_of(std::next(_model->basisFcts.cbegin()), _model->basisFcts.cend(),
                   [this](const std::vector<float>& f) {
                       return f.size() != _model->basisFcts.at(0).size();
                   }))
    {
        throw std::runtime_error(errMsgInconsistentBasisFcts());
    }

    this->setTime(0.0); // only for initializing the volume, otherwise the volume is empty
}

XYDataSeries BasisFunctionVolume::timeCurveNativeSampling(uint x, uint y, uint z) const
{
    XYDataSeries ret;

    const auto values = timeCurveValuesNativeSampling(x, y, z);
    size_t sampleCount = 0;

    for(auto val : values)
        ret.append(sample2Time(sampleCount++), val);

    return ret;
}

std::vector<float> BasisFunctionVolume::timeCurveValuesNativeSampling(uint x, uint y, uint z) const
{
    auto tac = std::inner_product(
        // iterate over all basis functions/coefficients
        _model->basisFcts.cbegin(), _model->basisFcts.cend(), _model->coeffVolumes.cbegin(),
        // init TAC function (zero function)
        std::vector<float>(_model->basisFcts.front().size(), 0.0f),
        // the "+" operation
        [](std::vector<float> f1, const std::vector<float>& f2) {
            std::transform(f1.cbegin(), f1.cend(), f2.cbegin(), f1.begin(), std::plus<float>());
            return f1;
        },
        // the "*" operation
        [x, y, z](std::vector<float> f, const VoxelVolume<float>& v) {
            const auto coeff = v(x, y, z);
            std::transform(f.cbegin(), f.cend(), f.begin(), [coeff](float val) { return coeff * val; });
            return f;
        });

    return tac;
}

// for now, sample = time [ms] (= view number, if not set otherwise)
float BasisFunctionVolume::sample2Time(size_t sample) const
{
    return static_cast<float>(sample);
}

size_t BasisFunctionVolume::time2Sample(double time) const
{
    return static_cast<size_t>(std::round(time));
}

std::string BasisFunctionVolume::errMsgDifferentNumberOfCoeffsAndBasisFcts() const
{
    std::string errorMsg("BasisFunctionVolume::BasisFunctionVolume: Number of coefficient volumes "
                         "does not match the number of basis functions.\n");
    errorMsg += "Number of basis functions: " + std::to_string(_model->basisFcts.size()) + "\n";
    errorMsg += "Number of coefficient volumes: " + std::to_string(_model->coeffVolumes.size());

    return errorMsg;
}

std::string BasisFunctionVolume::errMsgInconsistentBasisFcts() const
{
    std::string errorMsg("BasisFunctionVolume::BasisFunctionVolume: Inconsistent length of "
                         "of basis functions.\n"
                         "Samples of basis functions:\n| ");

    std::for_each(_model->basisFcts.cbegin(), _model->basisFcts.cend(),
                  [&errorMsg](const std::vector<float>& f) {
                      errorMsg += std::to_string(f.size()) + " | ";
                  });

    return errorMsg;
}

std::string BasisFunctionVolume::errMsgInconsistentVolumes() const
{
    std::string errorMsg("BasisFunctionVolume::BasisFunctionVolume: Inconsistent voxel size "
                         "or dimensions of coefficient volumes. "
                         "Dimensions of coefficient volumes:\n");

    std::for_each(_model->coeffVolumes.cbegin(), _model->coeffVolumes.cend(),
                  [&errorMsg](const VoxelVolume<float>& v) {
                      errorMsg += v.nbVoxels().info() + " | " + v.voxelSize().info() + "\n";
                  });

    return errorMsg;
}

} // namespace CTL
