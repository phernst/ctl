#include "detectorsaturationextension.h"
#include "components/abstractdetector.h"

namespace CTL {

void DetectorSaturationExtension::configure(const AcquisitionSetup &setup, const AbstractProjectorConfig &config)
{
    _setup = setup;

    ProjectorExtension::configure(setup, config);
}

ProjectionData DetectorSaturationExtension::project(const VolumeData &volume)
{
    auto ret = ProjectorExtension::project(volume);

    auto saturationModelType = _setup.system()->detector()->saturationModelType();

    if(saturationModelType == AbstractDetector::Extinction)
        processExtinctions(&ret);

    return ret;
}

void DetectorSaturationExtension::processExtinctions(ProjectionData *projections)
{
    auto saturationModel = _setup.system()->detector()->saturationModel();

    for(auto& view : projections->data())
        for(auto& module : view.data())
            for(auto& pix : module.data())
                pix = saturationModel->valueAt(pix);
}

}
