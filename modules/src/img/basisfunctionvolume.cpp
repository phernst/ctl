#include "basisfunctionvolume.h"
#include <QDebug>

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
        qDebug() << "Number of basis functions:" << _model->basisFcts.size()
                 << "Number of coefficient volumes:" << _model->coeffVolumes.size();
        throw std::runtime_error("BasisFunctionVolume::BasisFunctionVolume: Number of coefficient "
                                 "volumes does not match the number of basis functions.");
    }

    if(std::any_of(std::next(_model->coeffVolumes.cbegin()), _model->coeffVolumes.cend(),
                   [this](const VoxelVolume& c) {
                       return c.nbVoxels() != this->nbVoxels()
                           || c.voxelSize() != this->voxelSize();
                   }))
    {
        auto errorMsg = QStringLiteral("Dimensions of coefficient volumes:\n");
        std::for_each(_model->coeffVolumes.cbegin(), _model->coeffVolumes.cend(),
                      [&errorMsg](const VoxelVolume<float>& v) {
                          errorMsg.append(v.nbVoxels().info().c_str());
                          errorMsg.append(" | ");
                          errorMsg.append(v.voxelSize().info().c_str());
                          errorMsg.append("\n");
                      });
        qDebug().noquote() << errorMsg;
        throw std::runtime_error("BasisFunctionVolume::BasisFunctionVolume: Inconsistent voxel "
                                 "size or dimensions of coefficient volumes.");
    }

    if(std::any_of(std::next(_model->basisFcts.cbegin()), _model->basisFcts.cend(),
                   [this](const std::vector<float>& f) {
                       return f.size() != _model->basisFcts.at(0).size();
                   }))
    {
        auto errorMsg = QStringLiteral("Samples of basis functions:\n| ");
        std::for_each(_model->basisFcts.cbegin(), _model->basisFcts.cend(),
                      [&errorMsg](const std::vector<float>& f) {
                          errorMsg.append(QString::number(f.size()));
                          errorMsg.append(" | ");
                      });
        qDebug().noquote() << errorMsg;
        throw std::runtime_error("BasisFunctionVolume::BasisFunctionVolume: Inconsistent length "
                                 "of basis functions.");
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

} // namespace CTL
