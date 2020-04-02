#include "basisfunctionvolume.h"
#if __cplusplus >= 201703L
#include <numeric>
#include <execution>
#endif

template <class... Args>
static auto innerProduct(Args&&... args)
    -> decltype(std::inner_product(std::forward<Args>(args)...))
{
#if __cplusplus >= 201703L
    return std::transform_reduce(std::execution::par, std::forward<Args>(args)...);
#else
    return std::inner_product(std::forward<Args>(args)...);
#endif
}

namespace CTL {

void BasisFunctionVolume::updateVolume()
{
    this->fill(0.0f);

    const auto discreteTime = time2Sample(this->time());

    if(discreteTime >= _basisFcts.front().size())
        return;

    auto updatedVol = innerProduct(
        // iterate over all coefficients/basis functions
        _coeffVolumes.cbegin(), _coeffVolumes.cend(), _basisFcts.cbegin(),
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
    : AbstractDynamicVolumeData(VoxelVolume<float>(coeffVolumes.at(0).nbVoxels(),
                                                   coeffVolumes.at(0).voxelSize()))
    , _coeffVolumes(std::move(coeffVolumes))
    , _basisFcts(std::move(basisFunctions))
{
    // check for consistent sizes
    if(_basisFcts.size() != _coeffVolumes.size())
        throw std::runtime_error("BasisFunctionVolume::BasisFunctionVolume: Number of coefficient "
                                 "volumes does not match the number of basis functions.");

    if(std::any_of(
           std::next(_coeffVolumes.cbegin()), _coeffVolumes.cend(), [this](const VoxelVolume& c) {
               return c.nbVoxels() != this->nbVoxels() || c.voxelSize() != this->voxelSize();
           }))
        throw std::runtime_error("BasisFunctionVolume::BasisFunctionVolume: Inconsistent voxel "
                                 "size or dimensions of coefficient volumes.");

    if(std::any_of(
           std::next(_basisFcts.cbegin()), _basisFcts.cend(),
           [this](const std::vector<float>& t) { return t.size() != _basisFcts.at(0).size(); }))
        throw std::runtime_error("BasisFunctionVolume::BasisFunctionVolume: Inconsistent length "
                                 "of basis functions.");

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
        _basisFcts.cbegin(), _basisFcts.cend(), _coeffVolumes.cbegin(),
        // init TAC function (zero function)
        std::vector<float>(_basisFcts.front().size(), 0.0f),
        // the "+" operation
        [](std::vector<float> f1, const std::vector<float>& f2) {
            std::transform(f1.cbegin(), f1.cend(), f2.cbegin(), f1.begin(),
                           [](float val1, float val2) { return val1 + val2; });
            return f1;
        },
        // the "*" operation
        [x, y, z](std::vector<float> f, const VoxelVolume<float>& v) {
            const auto coeff = v(x, y, z);
            std::transform(f.cbegin(), f.cend(), f.begin(),
                           [coeff](float val) { return coeff * val; });
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
