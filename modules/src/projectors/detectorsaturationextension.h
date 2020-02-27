#ifndef DETECTORSATURATIONEXTENSION_H
#define DETECTORSATURATIONEXTENSION_H

#include "acquisition/acquisitionsetup.h"
#include "projectorextension.h"

namespace CTL {

class DetectorSaturationExtension : public ProjectorExtension
{
    CTL_TYPE_ID(102)

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;

public:
    DetectorSaturationExtension() = default;
    using ProjectorExtension::ProjectorExtension;
    explicit DetectorSaturationExtension(uint nbSpectralSamples);

    // ProjectorExtension interface
    bool isLinear() const override;
    void setIntensitySampling(uint nbSamples);

    // SerializationInterface interface
    QVariant toVariant() const override;
    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

protected:
    ProjectionData extendedProject(const MetaProjector& nestedProjector) override;

private:
    void processCounts(ProjectionData& projections);
    void processExtinctions(ProjectionData& projections);
    void processIntensities(ProjectionData& projections);

    AcquisitionSetup _setup;
    uint _nbSamples = 0;
};

} // namespace CTL

#endif // DETECTORSATURATIONEXTENSION_H
