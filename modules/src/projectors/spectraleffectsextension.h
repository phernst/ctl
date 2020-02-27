#ifndef SPECTRALEFFECTSEXTENSION_H
#define SPECTRALEFFECTSEXTENSION_H

#include "projectorextension.h"
#include "abstractprojectorconfig.h"
#include "acquisition/acquisitionsetup.h"
#include "processing/coordinates.h" // Range<T>
#include "acquisition/radiationencoder.h"

namespace CTL {

class SpectralEffectsExtension : public ProjectorExtension
{
    CTL_TYPE_ID(104)

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;
    public: ProjectionData project(const VolumeData& volume) override;

public:
    SpectralEffectsExtension() = default;
    explicit SpectralEffectsExtension(float energyBinWidth);
    using ProjectorExtension::ProjectorExtension;

    ProjectionData projectComposite(const CompositeVolume& volume) override;
    bool isLinear() const override;

    // SerializationInterface interface
    QVariant toVariant() const override;
    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

    void setSpectralSamplingResolution(float energyBinWidth);

private:  
    void updateSpectralInformation();
    bool canBypassExtension(const CompositeVolume& volume) const;
    void applyDetectorResponse(ProjectionData& intensity, float energy) const;

    using BinInformation = SpectralInformation::BinInformation;

    ProjectionData projectLinear(const CompositeVolume& volume);
    ProjectionData projectNonLinear(const CompositeVolume& volume);
    ProjectionData singleBinIntensityLinear(const std::vector<ProjectionData>& materialProjs,
                                            const std::vector<float>& mu,
                                            const BinInformation& binInfo);
    ProjectionData singleBinIntensityNonLinear(const CompositeVolume& volume,
                                               const BinInformation& binInfo);

    void addDummyPrepareSteps();
    void removeDummyPrepareSteps();
    void replaceDummyPrepareSteps(const BinInformation& binInfo, float binWidth);

    SpectralInformation _spectralInfo;
    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    float _deltaE = 0.0f;
};


} // namespace CTL

#endif // SPECTRALEFFECTSEXTENSION_H
