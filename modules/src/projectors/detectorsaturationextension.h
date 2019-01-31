#ifndef DETECTORSATURATIONEXTENSION_H
#define DETECTORSATURATIONEXTENSION_H

#include "acquisition/acquisitionsetup.h"
#include "projectorextension.h"

namespace CTL {

class DetectorSaturationExtension : public ProjectorExtension
{
public:
    using ProjectorExtension::ProjectorExtension;

    // ProjectorExtension interface
    void configure(const AcquisitionSetup &setup, const AbstractProjectorConfig &config) override;
    ProjectionData project(const VolumeData &volume) override;

private:
    AcquisitionSetup _setup;

    void processExtinctions(ProjectionData* projections);
    void processIntensities(ProjectionData* projections);
};



}

#endif // DETECTORSATURATIONEXTENSION_H
