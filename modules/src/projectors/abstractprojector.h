#ifndef CTL_ABSTRACTPROJECTOR_H
#define CTL_ABSTRACTPROJECTOR_H

#include "img/compositevolume.h"
#include "img/projectiondata.h"
#include "io/serializationinterface.h"

#include <QObject>
#include <memory>

/*
 * NOTE: This is header only.
 */

namespace CTL {

// forward declarations
class AcquisitionSetup;

/*!
 * Alias name for SpectralVolumeData.
 *
 * Also serves as a placeholder for potential future changes to the concept of volume data.
 */
typedef SpectralVolumeData VolumeData;

/*!
 * \class AbstractProjector
 *
 * \brief The AbstractProjector class is the abstract base class defining the interfaces for forward
 * projectors.
 *
 * This class defines the interface every forward projection implementation needs to satisfy. This
 * comes down to two methods that need to be provided:
 * - configure(): This method takes the AcquisitionSetup to be used for the simulation. All
 * necessary information to prepare the actual forward projection should be gathered here. This
 * usually contains all geometry and system information (which can be retrieved from the passed
 * AcquisitionSetup). Implementation specific parameters (e.g. accuracy settings), however, shall be
 * set using dedicated setter methods.
 * - project(): This method must provide the actual forward projection functionality. It takes the
 * voxelized volume that shall be projected and must return the full set of forward projections
 * that have been requested by the AcquisitionSetup set in the configure() step.
 *
 * Two structurally different ways of how such an implementation can be realized are given by
 * the examples RayCasterAdapter and RayCasterProjector. Both can be found in the OpenCL module
 * (ocl_routines.pri).
 */

/*!
 * \class ProjectorNotifier
 *
 * \brief Helper class that can emit signals during calculations of a certain projector.
 *
 * This class uses Qt's signal/slot principle to allow communication of a projector class with other
 * program parts. To do so, connect the signals of this object with the desired receiver objects.
 *
 * The ProjectorNotifier offers the following signals:
 * - projectionFinished(int): intended to be emitted when a projection is fully processed.
 * - information(QString): used to communicate status information.
 */

class ProjectorNotifier : public QObject
{
    Q_OBJECT
signals:
    void projectionFinished(int viewNb);
    void information(QString info);
};

class AbstractProjector : public SerializationInterface
{
    CTL_TYPE_ID(0)

    // abstract interface
    public:virtual void configure(const AcquisitionSetup& setup) = 0;
    public:virtual ProjectionData project(const VolumeData& volume) = 0;

public:
    virtual ~AbstractProjector() override = default;

    virtual bool isLinear() const;
    virtual ProjectionData projectComposite(const CompositeVolume& volume);

    void fromVariant(const QVariant& variant) override;
    QVariant toVariant() const override;
    virtual QVariant parameter() const;
    virtual void setParameter(const QVariant& parameter);

    virtual ProjectorNotifier* notifier();

protected:
    AbstractProjector() = default;
    AbstractProjector(const AbstractProjector&) = delete;
    AbstractProjector(AbstractProjector&&) = delete;
    AbstractProjector& operator=(const AbstractProjector&) = delete;
    AbstractProjector& operator=(AbstractProjector&&) = delete;

private:
    ProjectorNotifier _notifier; //!< The notifier object used for signal emission.
};

inline bool AbstractProjector::isLinear() const { return true; }

inline ProjectionData AbstractProjector::projectComposite(const CompositeVolume& volume)
{
    if(volume.isEmpty())
        throw std::runtime_error("AbstractProjector::projectComposite: Volume is empty.");

    // project first sub volume
    ProjectionData ret = project(volume.subVolume(0));

    // project remaining sub volumes
    for(auto subVol = 1u, nbSubVol = volume.nbSubVolumes(); subVol < nbSubVol; ++subVol)
        ret += project(volume.subVolume(subVol));

    return ret;
}

inline QVariant AbstractProjector::parameter() const { return QVariant(); }

inline void AbstractProjector::setParameter(const QVariant&) {}

inline QVariant AbstractProjector::toVariant() const
{
    QVariantMap ret = SerializationInterface::toVariant().toMap();

    ret.insert("parameters", parameter());

    return ret;
}

inline void AbstractProjector::fromVariant(const QVariant& variant)
{
    SerializationInterface::fromVariant(variant);

    const auto map = variant.toMap();

    setParameter(map.value("parameters").toMap());
}

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
* \fn void ProjectorNotifier::information(QString info)
*
* Signal that can be emitted to communicate a status information.
*/

/*!
 * \fn AbstractProjector::~AbstractProjector()
 *
 * Virtual default destructor.
 */

/*!
 * \fn void AbstractProjector::configure(const AcquisitionSetup& setup)
 *
 * \brief Configures the projector.
 *
 * This method should be used to gather all necessary information to prepare the actual forward
 * projection. This usually contains all geometry and system information, which can be retrieved
 * from \a setup.
 */

/*!
 * \fn ProjectionData AbstractProjector::project(const VolumeData& volume)
 *
 * \brief Provides the actual forward projection functionality.
 *
 * This method takes a voxelized dataset \a volume and shall return the full set of forward
 * projections that have been requested by the AcquisitionSetup set in the configure() step.
 *
 * The passed volume data can be either:
 * - SpectralVolumeData,
 * - VoxelVolume<float> (implicitely converted to SpectralVolumeData),
 * - any sub-class of AbstractDynamicVolumeData.
 *
 * CompositeVolume data can be projected using projectComposite().
 *
 * Note that the functionality of specific ProjectorExtension classes might depend on a passing a
 * certain type of volume data. Please refer to the documentation of the extensions you are using.
 */

/*!
* \fn ProjectionData AbstractProjector::projectComposite(const CompositeVolume& volume)
*
* \brief Provides the functionality to forward project CompositeVolume data.
*
* This method takes a composite dataset \a volume and returns the full set of forward projections
* according to the AcquisitionSetup set in the configure() step.
*
* By default, this method performs separate calls to project() for each individual voxel volume
* stored in the composite \a volume. The final projection result is the sum of all these individual
* projections (extinction domain).
* Change this behavior in sub-classes, if this is not suitable for your desired purpose. This is
* typically the case for non-linear operations.
*/

/*!
* \fn bool AbstractProjector::isLinear() const
*
* \brief Returns true if the projection operation is linear.
*
* By default, this method returns true. Override this method to return false in case of sub-classing
* that leads to non-linear operations. Overrides of this method should never return an unconditional
* \c true (as this might outrule underlying non-linearity).
*/

/*!
* \fn bool AbstractProjector::fromVariant(const QVariant& variant)
*
* Implementation of the deserialization interface. Sets the contents of the object based on the
* QVariant \a variant.
*
* This method uses setParameter() to deserialize class members.
*/

/*!
* \fn bool AbstractProjector::toVariant() const
*
* Implementation of the serialization interface. Stores the contents of this instance in a QVariant.
*
* Stores the object's type-id (from SerializationInterface::toVariant()).
*
* This method uses parameter() to serialize class members.
*/

/*!
* \fn bool AbstractProjector::parameter() const
*
* Returns the parameters of this instance as QVariant.
* This shall return a QVariantMap with key-value-pairs representing all settings of the object.
*
* This method is used within toVariant() to serialize the object's settings.
*/

/*!
* \fn bool AbstractProjector::setParameter(const QVariant& parameter)
*
* Sets the parameters of this instance based on the passed QVariant \a parameter. Parameters need
* to follow the naming convention as described in parameter().
*
* This method is used within fromVariant() to deserialize the object's settings. Direct use of
* this method is discouraged; consider using dedicated setter methods instead.
*/
} // namespace CTL

/*! \file */
///@{
/*!
 * \typedef typedef SpectralVolumeData VolumeData;
 *
 * \brief Alias name for SpectralVolumeData.
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

#endif // CTL_ABSTRACTPROJECTOR_H
