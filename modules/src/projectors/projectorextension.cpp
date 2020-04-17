#include "projectorextension.h"
#include <QDebug>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(ProjectorExtension)

ProjectorExtension::MetaProjector::MetaProjector(const VolumeData& volume,
                                                        AbstractProjector* projector)
    : _projector(projector)
    , _simpleVolume(&volume)
    , _compositeVolume(nullptr)
{
}

ProjectorExtension::MetaProjector::MetaProjector(const CompositeVolume& volume,
                                                        AbstractProjector* projector)
    : _projector(projector)
    , _simpleVolume(nullptr)
    , _compositeVolume(&volume)
{
}

bool ProjectorExtension::MetaProjector::isComposite() const
{
    return _compositeVolume;
}

ProjectionData ProjectorExtension::MetaProjector::project() const
{
    return isComposite() ? _projector->projectComposite(*_compositeVolume)
                         : _projector->project(*_simpleVolume);
}

ProjectionData ProjectorExtension::extendedProject(const MetaProjector& nestedProjector)
{
    qDebug() << "called metaProject";
    return nestedProjector.project();
}

/*!
 * Constructs a ProjectorExtension object and sets the nested projector to \a projector.
 * The nested projector is internally used as a basis for computing forward projections.
 * Note that the constructed object takes over the ownership of \a projector.
 */
ProjectorExtension::ProjectorExtension(AbstractProjector* projector)
    : _projector( (projector==this) ? nullptr : projector )
{
    if(projector==this)
        qWarning() << "ProjectorExtension::ProjectorExtension(): Tried to pass the object itself "
                      "to the constructor. Nested projector is set to nullptr.";
    if(_projector)
    {
        QObject::connect(_projector->notifier(), &ProjectorNotifier::projectionFinished,
                         this->notifier(), &ProjectorNotifier::projectionFinished);
        QObject::connect(_projector->notifier(), &ProjectorNotifier::information,
                         this->notifier(), &ProjectorNotifier::information);

    }
}

/*!
 * Constructs a ProjectorExtension object and sets the nested projector to \a projector.
 * A good practice to create a ProjectorExtension on the heap is to use the make function
 * makeExtension(std::unique_ptr<AbstractProjector> projector)
 * which will interally use this constructor.
 */
ProjectorExtension::ProjectorExtension(std::unique_ptr<AbstractProjector> projector)
    : ProjectorExtension(projector.release())
{
}

/*!
 * Destructs this instance. Calls the destructor of the nested projector object.
 */
ProjectorExtension::~ProjectorExtension() { delete _projector; }

/*!
 * This overrides the configure() method and calls the configure method of the nested projector
 * object.
 *
 * Re-implement this method to retrieve all information required for the purpose of the desired
 * extension. Make sure to delegate this call to the base class (ProjectorExtension) at the end of
 * the method.
 *
 * Throws std::runtime_error if the nested projector object is unset.
 */
void ProjectorExtension::configure(const AcquisitionSetup& setup)
{
    Q_ASSERT(_projector);
    if(!_projector)
        throw std::runtime_error("ProjectorExtension::configure(): no nested projector set.");

    _projector->configure(setup);
}

/*!
 * This overrides the project() method and calls the project method of the nested projector
 * object.
 *
 * Re-implement this method to modify the projections in order to realize the desired
 * functionality of your extension.
 *
 * Throws std::runtime_error if the nested projector object is unset.
 */
ProjectionData ProjectorExtension::project(const VolumeData& volume)
{
    Q_ASSERT(_projector);
    if(!_projector)
        throw std::runtime_error("ProjectorExtension::project(): no nested projector set.");

    qDebug() << "build MetaProjector";
    MetaProjector p(volume, _projector);
    qDebug() << "MetaProjector rdy";

    return extendedProject(p);
}

ProjectionData ProjectorExtension::projectComposite(const CompositeVolume& volume)
{
    Q_ASSERT(_projector);
    if(!_projector)
        throw std::runtime_error("ProjectorExtension::projectComposite(): no nested projector set.");

    MetaProjector p(volume, _projector);

    return extendedProject(p);
}

bool ProjectorExtension::isLinear() const
{
    Q_ASSERT(_projector);
    if(!_projector)
        throw std::runtime_error("ProjectorExtension::isLinear(): no nested projector set.");

    return _projector->isLinear();
}

// Use SerializationInterface::fromVariant() documentation.
void ProjectorExtension::fromVariant(const QVariant& variant)
{
    AbstractProjector::fromVariant(variant);

    QVariantMap map = variant.toMap();

    if(!map.value("nested projector").isNull())
        use(SerializationHelper::parseProjector(map.value("nested projector")));
}

// Use SerializationInterface::toVariant() documentation.
QVariant ProjectorExtension::toVariant() const
{
    QVariantMap ret = AbstractProjector::toVariant().toMap();

    ret.insert("nested projector",
               _projector ? _projector->toVariant() : QVariant());

    return ret;
}

/*!
 * Releases the nested projector object.
 *
 * This transfers the ownership of the projector object to the caller. The internal nested projector
 * pointer is set to nullptr.
 */
AbstractProjector* ProjectorExtension::release()
{
    AbstractProjector* ret = _projector;
    if(_projector)
        _projector->notifier()->disconnect();
    _projector = nullptr;
    return ret;
}

/*!
 * Resets this instance. This deletes the nested projector object and sets the internal pointer to
 * nullptr.
 */
void ProjectorExtension::reset()
{
    delete _projector;
    _projector = nullptr;
}

/*!
 * Sets the nested projector to \a other.
 *
 * This will overwrite any projector object that is already in place by deleting it. If this is
 * unintended, consider retrieving the nested projector first using release().
 * Note that it takes over the ownership of \a projector.
 */
void ProjectorExtension::use(AbstractProjector* other)
{
    if(other == this)
    {
        qWarning() << "ProjectorExtension::use(): Tried to 'use' the object itself. Nested projector"
                      " is set to nullptr.";
        other = nullptr;
    }
    delete _projector;
    _projector = other;
    if(_projector)
    {
        QObject::connect(_projector->notifier(), &ProjectorNotifier::projectionFinished,
                         this->notifier(), &ProjectorNotifier::projectionFinished);
        QObject::connect(_projector->notifier(), &ProjectorNotifier::information,
                         this->notifier(), &ProjectorNotifier::information);
    }
}

/*!
 * Overload of use(AbstractProjector* other) that takes a std::unique_ptr of the \a other projector.
 */
void ProjectorExtension::use(std::unique_ptr<AbstractProjector> other)
{
    this->use(other.release());
}

// pipe-assignment operators
// u_ptr, u_ptr
std::unique_ptr<AbstractProjector>& operator|=(std::unique_ptr<AbstractProjector>& lhs,
                                                      std::unique_ptr<ProjectorExtension> rhs)
{
    lhs = std::move(lhs) | std::move(rhs);
    return lhs;
}

std::unique_ptr<ProjectorExtension>& operator|=(std::unique_ptr<ProjectorExtension>& lhs,
                                                      std::unique_ptr<ProjectorExtension> rhs)
{
    lhs = std::move(lhs) | std::move(rhs);
    return lhs;
}

// u_ptr, raw_ptr
std::unique_ptr<AbstractProjector>& operator|=(std::unique_ptr<AbstractProjector>& lhs,
                                                      ProjectorExtension* rhs)
{
    lhs = std::move(lhs) | rhs;
    return lhs;
}

std::unique_ptr<ProjectorExtension>& operator|=(std::unique_ptr<ProjectorExtension>& lhs,
                                                      ProjectorExtension* rhs)
{
    lhs = std::move(lhs) | rhs;
    return lhs;
}

// raw_ptr, u_ptr
AbstractProjector*& operator|=(AbstractProjector*& lhs,
                                      std::unique_ptr<ProjectorExtension> rhs)
{
    lhs = (lhs | std::move(rhs)).release();
    return lhs;
}

ProjectorExtension*& operator|=(ProjectorExtension*& lhs,
                                      std::unique_ptr<ProjectorExtension> rhs)
{
    lhs = (lhs | std::move(rhs)).release();
    return lhs;
}

// raw_ptr, raw_ptr
AbstractProjector*& pipe(AbstractProjector*& lhs, ProjectorExtension* rhs)
{
    rhs->use(lhs);
    lhs = rhs;
    return lhs;
}

ProjectorExtension*& pipe(ProjectorExtension*& lhs, ProjectorExtension* rhs)
{
    rhs->use(lhs);
    lhs = rhs;
    return lhs;
}

}
