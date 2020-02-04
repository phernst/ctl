#ifndef STANDARDPIPELINE_H
#define STANDARDPIPELINE_H

#include "projectionpipeline.h"

namespace CTL {

namespace OCL {
    class RayCasterProjector;
}
class ArealFocalSpotExtension;
class DetectorSaturationExtension;
class PoissonNoiseExtension;
class SpectralEffectsExtension;

class StandardPipeline: public AbstractProjector
{
    CTL_TYPE_ID(201)

public:
    using ProjectorPtr = std::unique_ptr<AbstractProjector>;
    using ExtensionPtr = std::unique_ptr<ProjectorExtension>;

    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;
    ProjectionData projectComposite(const CompositeVolume &volume) override;
    bool isLinear() const override;

    // SerializationInterface interface
    void fromVariant(const QVariant& variant) override;
    QVariant toVariant() const override;

    StandardPipeline();
    ~StandardPipeline();

    // configuration methods
    void enableArealFocalSpot(bool enable);
    void enableDetectorSaturation(bool enable);
    void enablePoissonNoise(bool enable);
    void enableSpectralEffects(bool enable);

    void setArealFocalSpotDiscretization(const QSize& discretization);
    void setSpectralEffectsSampling(float energyBinWidth);

private:
    ProjectionPipeline _pipeline;

    bool _arealFSEnabled = false;
    bool _detSatEnabled = false;
    bool _spectralEffEnabled = false;
    bool _poissonEnabled = false;

    OCL::RayCasterProjector* _projector;
    ArealFocalSpotExtension* _extAFS;
    DetectorSaturationExtension* _extDetSat;
    PoissonNoiseExtension* _extPoisson;
    SpectralEffectsExtension* _extSpectral;

    uint posAFS() const;
    uint posDetSat() const;
    uint posPoisson() const;
    uint posSpectral() const;
};

}
#endif // STANDARDPIPELINE_H
