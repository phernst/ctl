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
    using ProjectorExtension::ProjectorExtension;

    // ProjectorExtension interface
    bool isLinear() const override;
    void setIntensitySampling(uint nbSamples);

    // SerializationInterface interface
    void fromVariant(const QVariant &variant) override;
    QVariant toVariant() const override;

protected:
    ProjectionData extendedProject(const MetaProjector& nestedProjector) override;

private:
    AcquisitionSetup _setup;
    uint _nbSamples = 0;

    void processCounts(ProjectionData& projections);
    void processExtinctions(ProjectionData& projections);
    void processIntensities(ProjectionData& projections);
};

} // namespace CTL

#endif // DETECTORSATURATIONEXTENSION_H
