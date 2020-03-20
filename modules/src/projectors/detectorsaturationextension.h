#ifndef CTL_DETECTORSATURATIONEXTENSION_H
#define CTL_DETECTORSATURATIONEXTENSION_H

#include "acquisition/acquisitionsetup.h"
#include "projectorextension.h"

namespace CTL {

/*!
 * \class DetectorSaturationExtension
 *
 * \brief The DetectorSaturationExtension class is an extension for forward projectors that 
 * considers over- and/or undersaturation effects of the detector.
 *
 * This extension performs a postprocessing on the projection data to consider over- and/or
 * undersaturation effects of the detector. It will use the saturation model set to the detector
 * component (see AbstractDetector::setSaturationModel()). Depending on the specification of the
 * saturation model, the postprocessing will be applied in the domain of extinction values,
 * intensities, or photon counts.
 * 
 * The following example shows how to extend a simple ray caster algorithm to consider detector
 * (over- and under-) saturation:
 * \code
 *   // define volume and acquisition setup (incl. system)
 *  auto volume = VolumeData::cube(100, 1.0f, 0.02f);
 *  auto system = SimpleCTsystem::fromCTsystem(CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT(DetectorBinning::Binning4x4)));
 *  AcquisitionSetup acquisitionSetup(system, 10);
 *  acquisitionSetup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0));
 *
 *  // Core part
 *
 *  // set a detector saturation model (operating in extinction domain, clamps values to [0.1, 2.5])
 *  auto saturationModel = new DetectorSaturationLinearModel(0.1f, 2.5f);
 *  acquisitionSetup.system()->detector()->setSaturationModel(saturationModel, AbstractDetector::Extinction);
 *
 *  auto simpleProjector = new RayCasterProjector; // our simple projector
 *      // optional parameter settings for the projector
 *      // e.g. simpleProjector->settings().raySampling = 0.1f;
 *
 *  // This is what we do without the extension:
 *      // simpleProjector->configure(acquisitionSetup);
 *      // ProjectionData projections = simpleProjector->project(volume);
 *      // qInfo() << projections.min() << projections.max(); // output: 0 2.79263
 *
 *  // Instead we now do the following:
 *  DetectorSaturationExtension* extension = new DetectorSaturationExtension;
 *
 *  extension->use(simpleProjector);                            // tell the extension to use the ray caster
 *  extension->configure(acquisitionSetup);                     // configure the simulation
 *
 *  ProjectionData projections = extension->project(volume);    // (compute and) get the final projections
 *
 *  qInfo() << projections.min() << projections.max();          // output: 0.1 2.5
 * \endcode
 */

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

    AcquisitionSetup _setup; //!< A copy of the acquisition setup.
    uint _nbSamples{ 0u };   //!< Number of samples used to extract spectrally resolved information.
};

} // namespace CTL

#endif // CTL_DETECTORSATURATIONEXTENSION_H
