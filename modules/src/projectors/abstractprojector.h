#ifndef ABSTRACTPROJECTOR_H
#define ABSTRACTPROJECTOR_H

#include "img/projectiondata.h"
#include "img/voxelvolume.h"

#include <QObject>
#include <memory>

/*
 * NOTE: This is header only.
 */

namespace CTL {

// forward declarations
class AcquisitionSetup;
class AbstractProjectorConfig;

/*!
 * Alias name for template specialization VoxelVolume<float>.
 *
 * Also serves as a placeholder for potential future changes to the concept of volume data.
 */
typedef VoxelVolume<float> VolumeData;

/*!
 * \class AbstractProjector
 *
 * \brief The AbstractProjector class is the abstract base class defining the interfaces for forward
 * projectors.
 *
 * This class defines the interface every forward projection implementation needs to satisfy. This
 * comes down to two methods that need to be provided.
 *
 * First, the configure() method is required.
 * It should be used to gather all necessary information to prepare the actual forward projection.
 * This usually contains all geometry and system information (which can be retrieved from the
 * passed AcquisitionSetup) and implementation specific parameters (which should be passed by a
 * AbstractProjectorConfig sub-classed for that particular purpose).
 *
 * Second, the actual forward projection functionality must be made available through project().
 * This method takes the voxelized volume that shall be projected and must return the full set
 * of forward projections that have been requested by the AcquisitionSetup set in the configurate()
 * step.
 *
 * Two structurally different ways of how such an implementation can be realized are given by
 * the examples RayCasterInterface and RayCasterProjector, which can be found in the OpenCL module
 * (ocl_projectors.pri).
 */

/*!
 * \class ProjectorNotifier
 *
 * \brief Helper class that can emit signals during calculations of a certain projector.
 *
 * This class uses Qt's signal/slot principle to allow communication of a projector class with other
 * program parts. To do so, connect the signals of this object with the desired receiver objects.
 */

class ProjectorNotifier : public QObject
{
    Q_OBJECT
signals:
    void projectionFinished(int viewNb);
};

class AbstractProjector
{
    // abstract interface
    public:virtual void configure(const AcquisitionSetup& setup,
                                  const AbstractProjectorConfig& config) = 0;
    public:virtual ProjectionData project(const VolumeData& volume) = 0;

public:
    virtual ~AbstractProjector() = default;

    ProjectorNotifier* notifier();

private:
    ProjectorNotifier _notifier; //!< The notifier object used for signal emission.
};

// factory function `makeProjector`
template <typename ProjectorType, typename... ConstructorArguments>
auto makeProjector(ConstructorArguments&&... arguments) ->
    typename std::enable_if<std::is_convertible<ProjectorType*, AbstractProjector*>::value,
                            std::unique_ptr<ProjectorType>>::type
{
    return std::unique_ptr<ProjectorType>(new ProjectorType(
                                              std::forward<ConstructorArguments>(arguments)...));
}

/*!
 * \brief
 * Returns a pointer to the notifier of the projector.
 *
 * The notifier object can be used to emit the signal
 * ProjectorNotifier::projectionFinished(int viewNb) when the calculation of the \a viewNb'th view
 * has been done.
 *
 * To receive emitted signals, use Qt's connect() method to connect the notifier object to any
 * receiver object of choice.
 *
 * Example - sending simulation progress information to a QProgressBar:
 * \code
 * AbstractProjector* myProjector;
 * // assume now that myProjector points at an object of a specific projector implementation
 *
 * QProgressBar* myProgressBar = new QProgressBar();
 * // ... e.g. add the progress bar somewhere in your GUI
 *
 * connect(myProjector->notifier(), SIGNAL(projectionFinished(int), myProgressBar, SLOT(setValue(int));
 * \endcode
 */
inline ProjectorNotifier* AbstractProjector::notifier() { return &_notifier; }

/*!
* \fn void ProjectorNotifier::projectionFinished(int viewNb)
*
* Signal that is emitted after processing of projection \a viewNb is finished.
*/

/*!
 * \fn AbstractProjector::~AbstractProjector()
 *
 * Virtual default destructor.
 */

/*!
 * \fn void AbstractProjector::configure(const AcquisitionSetup& setup, const
 * AbstractProjectorConfig& config)
 *
 * \brief Configures the projector.
 *
 * This method should be used to gather all necessary information to prepare the actual forward
 * projection. This usually contains all geometry and system information (which can be retrieved
 * from \a setup) and implementation specific parameters from \a config. To do so, consider
 * sub-classing AbstractProjectorConfig for the particular purpose.
 */

/*!
 * \fn ProjectionData AbstractProjector::project(const VolumeData& volume)
 *
 * \brief Provides the actual forward projection functionality.
 *
 * This method takes a voxelized dataset \a volume. It shall return the full set of forward
 * projections that have been requested by the AcquisitionSetup set in the configure() step.
 */

} // namespace CTL

/*! \file */
///@{
/*!
 * \typedef typedef VoxelVolume<float> CTL::VolumeData
 *
 * \brief Alias name for VoxelVolume<float>.
 *
 * \relates CTL::AbstractProjector
 */

/*!
 * \fn std::unique_ptr<ProjectorType> CTL::makeProjector(ConstructorArguments&&... arguments)
 * \relates AbstractProjector
 *
 * Global (free) make function that creates a new Projector from possible constructor \a arguments.
 * The component is returned as a `std::unique_ptr<ProjectorType>`, whereas `ProjectorType` is the
 * template argument of this function that needs to be specified.
 */
///@}

#endif // ABSTRACTPROJECTOR_H
