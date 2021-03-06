#ifndef CTL_AREALFOCALSPOTEXTENSION_H
#define CTL_AREALFOCALSPOTEXTENSION_H

#include "projectorextension.h"
#include "acquisition/acquisitionsetup.h"
#include <QPointF>
#include <QSize>

namespace CTL {
/*!
 * \class ArealFocalSpotExtension
 *
 * \brief The ArealFocalSpotExtension class is an extension for forward projectors that considers
 * the finite dimensions of the focal spot.
 *
 * This class is an extension for projectors that takes into account the finite extent of the
 * X-ray source. Typically, forward projection routines assume a point source, which is only
 * an approximation of the real scenario.
 *
 * In reality, the origin of the radiation - henceforth
 * referred to as the focal spot - covers the area of
 * \f$\left[-\frac{w_{fs}}{2},\frac{w_{fs}}{2}\right]\times
 * \left[-\frac{h_{fs}}{2},\frac{h_{fs}}{2}\right]\f$, where \f$w_{fs}\f$ and \f$h_{fs}\f$ denote
 * the width and height of the focal spot, respectively. To simulate this extended area, projections
 * will be simulated for a grid of sampling points (see figure below) and averaged afterwards. The
 * number of discretization steps can be specified using setDiscretization().
 *
 * By default, the focal spot discretization defaults to [1, 1], resulting in conventional point
 * source approximation.
 *
 * ![Illustration of the focal spot discretization principle.](focalSpotExtension.png)
 *
 * Note that this extension will increase the time required for projection linearly with the number
 * of requested sampling points. It also doubles the required system memory (needs to keep two full
 * sets of projections in memory simultaneously).
 *
 * The following example shows how to extend a simple ray caster algorithm to approximate the focal
 * spot extensions with a 5x5 grid:
 * \code
 *  // some definitions
 *  VolumeData volume;
 *  // ...
 *  AcquisitionSetup acquisitionSetup;
 *  // ...
 *
 *  // Core part
 *  auto simpleProjector = new RayCasterProjector; // our simple projector
 *      // optional parameter settings for the projector
 *      // e.g. simpleProjector->settings().raySampling = 0.1f;
 *
 *  // this is what you do without extension:
 *      // simpleProjector->configure(acquisitionSetup);
 *      // ProjectionData projections = simpleProjector->project(volume);
 *
 *  // instead we now do the following
 *  ArealFocalSpotExtension* extension = new ArealFocalSpotExtension;
 *
 *  extension->use(simpleProjector);                            // tell the extension to use the ray caster
 *  extension->setDiscretization(QSize(5, 5));                  // set discretization grid to 5x5 points
 *  extension->configure(acquisitionSetup);                     // configure the simulation
 *
 *  ProjectionData projections = extension->project(volume);    // (compute and) get the final projections
 * \endcode
 */
class ArealFocalSpotExtension : public ProjectorExtension
{
    CTL_TYPE_ID(101)

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;

public:
    ArealFocalSpotExtension() = default;
    using ProjectorExtension::ProjectorExtension;
    explicit ArealFocalSpotExtension(const QSize& discretization,
                                     bool lowExtinctionApproximation = false);

    void setDiscretization(const QSize& discretization);
    void enableLowExtinctionApproximation(bool enable = true);

    // SerializationInterface interface
    QVariant toVariant() const override;
    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;
    bool isLinear() const override;

protected:
    ProjectionData extendedProject(const MetaProjector& nestedProjector) override;
    QVector<QPointF> discretizationGrid() const;

    QSize _discretizationSteps{ 1, 1 }; //!< Requested number of discretization steps in both dimensions.
    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    bool _lowExtinctionApprox{ false }; //!< True if low attenuation approximation has been enabled.
};

} // namespace CTL

/*! \file */

#endif // CTL_AREALFOCALSPOTEXTENSION_H
