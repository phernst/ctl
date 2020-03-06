#ifndef DETECTORSATURATIONEXTENSION_H
#define DETECTORSATURATIONEXTENSION_H

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
 * The following example shows how to extend a simple ray caster algorithm to consider a detector
 * saturation extension:
 * \code
 *
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
    AcquisitionSetup _setup; //!< A copy of the acquisition setup.
    uint _nbSamples = 0;     //!< Number of samples used to extract spectrally resolved information.

    void processCounts(ProjectionData& projections);
    void processExtinctions(ProjectionData& projections);
    void processIntensities(ProjectionData& projections);
};

} // namespace CTL

#endif // DETECTORSATURATIONEXTENSION_H
