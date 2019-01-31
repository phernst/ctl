#include "detectorsaturationextension.h"
#include "components/abstractdetector.h"
#include "components/abstractsource.h"

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

    switch (saturationModelType) {
    case AbstractDetector::Extinction:
        processExtinctions(&ret);
        break;
    case AbstractDetector::Intensity:
        processIntensities(&ret);
        break;
    case AbstractDetector::Undefined:
        qWarning() << "DetectorSaturationExtension::project(): Undefined saturation model!";
        break;
    }

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
    futures.reserve(_setup.nbViews());
    for(auto& view : projections->data())
        futures.push_back(std::async(processView, &view));

//    //sequential
//    for(auto& view : projections->data())
//        for(auto& module : view.data())
//            for(auto& pix : module.data())
//                pix = saturationModel->valueAt(pix);    // pass pixel value through saturation model
}

void DetectorSaturationExtension::processIntensities(ProjectionData *projections)
{
    auto saturationModel = _setup.system()->detector()->saturationModel();
    auto sourcePtr = _setup.system()->source();

    auto processView = [saturationModel](SingleViewData* view, int i0)
    {
        float intensity;
        for(auto& module : view->data())
            for(auto& pix : module.data())
            {
                // transform extinction to intensity
                intensity = i0 * exp(-pix);
                // pass intensity through saturation model
                intensity = saturationModel->valueAt(intensity);
                // back-transform to extinction and overwrite projection pixel value
                pix = log(i0 / intensity);
            }
    };

    std::vector<std::future<void>> futures;
    futures.reserve(_setup.nbViews());
    int v = 0;
    float i0;
    for(auto& view : projections->data())
    {
        _setup.prepareView(v++);
        i0 = sourcePtr->photonFlux();

        futures.push_back(std::async(processView, &view, i0));
    }
}

}
