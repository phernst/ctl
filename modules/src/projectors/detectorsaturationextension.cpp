#include "detectorsaturationextension.h"
#include "components/abstractdetector.h"

#include <future>

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

    auto processView = [saturationModel](SingleViewData* view)
    {
        for(auto& module : view->data())
            for(auto& pix : module.data())
                pix = saturationModel->valueAt(pix);
    };

    std::vector<std::future<void>> futures;
    for(auto& view : projections->data())
        futures.emplace_back(std::async(processView, &view));

//    //sequential
//    for(auto& view : projections->data())
//        for(auto& module : view.data())
//            for(auto& pix : module.data())
//                pix = saturationModel->valueAt(pix);    // pass pixel value through saturation model
}



}
