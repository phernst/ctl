#ifndef AREALFOCALSPOTEXTENSION_H
#define AREALFOCALSPOTEXTENSION_H

#include "projectorextension.h"
#include "abstractprojectorconfig.h"
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
 *  RayCasterProjector::Config projectorConfig;
 *  // ...
 *
 *  // Core part
 *  AbstractProjector* simpleProjector = new RayCasterProjector; // our simple projector
 *
 *  // this is what you do without extension:
 *      // simpleProjector->configurate(acquisitionSetup, projectorConfig);
 *      // ProjectionData projections = simpleProjector->project(volume);
 *
 * // instead we now do the following
 *  ArealFocalSpotExtension* extendedRayCaster = new ArealFocalSpotExtension;
 *
 *  extendedRayCaster->use(simpleProjector);                            // tell the extension to use the ray caster
 *  extendedRayCaster->setDiscretization(QSize(5, 5));                  // set discretization grid to 5x5 points
 *  extendedRayCaster->configurate(acquisitionSetup, projectorConfig);  // configurate the extension
 *
 *  ProjectionData projections = extendedRayCaster->project(volume);    // (compute and) get the final projections
 * \endcode
 */
class ArealFocalSpotExtension : public ProjectorExtension
{
public:
    using ProjectorExtension::ProjectorExtension;

    void configure(const AcquisitionSetup& setup, const AbstractProjectorConfig& config) override;
    ProjectionData project(const VolumeData& volume) override;

    void setDiscretization(const QSize& discretization);

protected:
    QSize _discretizationSteps = { 1, 1 }; //!< Requested number of discretization steps in both dimensions.
    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    std::unique_ptr<AbstractProjectorConfig> _config; //!< A copy of the projector configuration.

    QVector<QPointF> discretizationGrid() const;
};

/*!
 * Sets the discretization of the focal spot to \a discretization. The focal spot will then be
 * sampled with \c discretization.width() x \c discretization.height() sampling points. These
 * sampling dimensions correspond to the width and height of the focal spot (or x and y direction
 * in CT coordinates).
 */
inline void ArealFocalSpotExtension::setDiscretization(const QSize& discretization)
{
    _discretizationSteps = discretization;
}

} // namespace CTL

/*! \file */

#endif // AREALFOCALSPOTEXTENSION_H
