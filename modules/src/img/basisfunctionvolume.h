#ifndef BASISFUNCTIONVOLUME_H
#define BASISFUNCTIONVOLUME_H

#include "abstractdynamicvolumedata.h"

namespace CTL {

class BasisFunctionVolume : public AbstractDynamicVolumeData
{
public:
    using CoeffVolumes = std::vector<VoxelVolume<float>>;
    using SampledFunctions = std::vector<std::vector<float>>;

    // abstract interface
    protected: void updateVolume() override;
    public: SpectralVolumeData* clone() const override;

public:
    BasisFunctionVolume(CoeffVolumes coeffVolumes, SampledFunctions basisFunctions);

    XYDataSeries timeCurveNativeSampling(uint x, uint y, uint z) const;
    std::vector<float> timeCurveValuesNativeSampling(uint x, uint y, uint z) const;

    float sample2Time(size_t sample) const;
    size_t time2Sample(double time) const;

    // generic function for arbitrary time points (e.g. by interpolation) could be overrided for
    // better efficiency (if required):
    //XYDataSeries timeCurve(uint x, uint y, uint z, const std::vector<float>& timePoints) override;

private:
    CoeffVolumes _coeffVolumes;
    SampledFunctions _basisFcts;
};

} // namespace CTL

#endif // BASISFUNCTIONVOLUME_H
