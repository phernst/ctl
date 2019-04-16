#ifndef PROJECTOREXTENSION_H
#define PROJECTOREXTENSION_H

#include "abstractprojector.h"
#include <memory>
#include <QDebug>

/*
 * NOTE: This is header only.
 */

namespace CTL {
/*!
 * \class ProjectorExtension
 *
 * \brief The ProjectorExtension class provides the interface to extend projectors with additional
 * functionality using the concept of decoration.
 *
 * \sa configure(), project().
 */
class ProjectorExtension : public AbstractProjector
{
public:
    ProjectorExtension(AbstractProjector* projector = nullptr);
    ProjectorExtension(std::unique_ptr<AbstractProjector> projector);

    ~ProjectorExtension() override;

    // virtual methods
    void configure(const AcquisitionSetup& setup, const AbstractProjectorConfig& config) override;
    ProjectionData project(const VolumeData& volume) override;
    bool isLinear() const override;

    // other methods
    AbstractProjector* release();
    void reset();
    void use(AbstractProjector* other);
    void use(std::unique_ptr<AbstractProjector> other);

private:
    AbstractProjector* _projector; //!< The nested projector object.
};

// factory function `makeExtension` (2 overloads, one for each ctor)
template <typename ProjectorExtensionType>
auto makeExtension(AbstractProjector* projector = nullptr) ->
typename std::enable_if<std::is_convertible<ProjectorExtensionType*, ProjectorExtension*>::value,
                        std::unique_ptr<ProjectorExtensionType>>::type
{
    return std::unique_ptr<ProjectorExtensionType>(new ProjectorExtensionType(projector));
}

template <typename ProjectorExtensionType>
auto makeExtension(std::unique_ptr<AbstractProjector> projector) ->
typename std::enable_if<std::is_convertible<ProjectorExtensionType*, ProjectorExtension*>::value,
                        std::unique_ptr<ProjectorExtensionType>>::type
{
    return std::unique_ptr<ProjectorExtensionType>(
        new ProjectorExtensionType(std::move(projector)));
}

/*!
 * Constructs a ProjectorExtension object and sets the nested projector to \a projector.
 * The nested projector is internally used as a basis for computing forward projections.
 * Note that the constructed object takes over the ownership of \a projector.
 */
inline ProjectorExtension::ProjectorExtension(AbstractProjector* projector)
    : _projector( (projector==this) ? nullptr : projector )
{
    if(projector==this)
        qWarning() << "ProjectorExtension::ProjectorExtension(): Tried to pass the object itself "
                      "to the constructor. Nested projector is set to nullptr.";
    if(_projector)
        QObject::connect(_projector->notifier(), &ProjectorNotifier::projectionFinished,
                         this->notifier(), &ProjectorNotifier::projectionFinished);
}

/*!
 * Constructs a ProjectorExtension object and sets the nested projector to \a projector.
 * A good practice to create a ProjectorExtension on the heap is to use the make function
 * makeExtension(std::unique_ptr<AbstractProjector> projector)
 * which will interally use this constructor.
 */
inline ProjectorExtension::ProjectorExtension(std::unique_ptr<AbstractProjector> projector)
    : ProjectorExtension(projector.release())
{
}

/*!
 * Destructs this instance. Calls the destructor of the nested projector object.
 */
inline ProjectorExtension::~ProjectorExtension() { delete _projector; }

/*!
 * This overrides the configure() method and calls the configure method of the nested projector
 * object.
 *
 * Re-implement this method to retrieve all information required for the purpose of the desired
 * extension. Make sure to delegate this call to the nested projector object at the end of the
 * method.
 *
 * Throws std::runtime_error if the nested projector object is unset.
 */
inline void ProjectorExtension::configure(const AcquisitionSetup& setup,
                                     const AbstractProjectorConfig& config)
{
    Q_ASSERT(_projector);
    if(!_projector)
        throw std::runtime_error("ProjectorExtension::configure(): no nested projector set.");

    _projector->configure(setup, config);
}

/*!
 * This overrides the project() method and calls the project method of the nested projector
 * object.
 *
 * Re-implement this method to modify the projections in order to realize the desired
 * functionality of your extension. Make sure to delegate this call to the nested projector
 * object at the end of the method.
 *
 * Throws std::runtime_error if the nested projector object is unset.
 */
inline ProjectionData ProjectorExtension::project(const VolumeData& volume)
{
    Q_ASSERT(_projector);
    if(!_projector)
        throw std::runtime_error("ProjectorExtension::project(): no nested projector set.");

    return _projector->project(volume);
}

inline bool ProjectorExtension::isLinear() const
{
    Q_ASSERT(_projector);
    if(!_projector)
        throw std::runtime_error("ProjectorExtension::isLinear(): no nested projector set.");

    return _projector->isLinear();
}

/*!
 * Releases the nested projector object.
 *
 * This transfers the ownership of the projector object to the caller. The internal nested projector
 * pointer is set to nullptr.
 */
inline AbstractProjector* ProjectorExtension::release()
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
inline void ProjectorExtension::reset()
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
inline void ProjectorExtension::use(AbstractProjector* other)
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
        QObject::connect(_projector->notifier(), &ProjectorNotifier::projectionFinished,
                         this->notifier(), &ProjectorNotifier::projectionFinished);
}

/*!
 * Overload of use(AbstractProjector* other) that takes a std::unique_ptr of the \a other projector.
 */
inline void ProjectorExtension::use(std::unique_ptr<AbstractProjector> other)
{
    this->use(other.release());
}

} // namespace CTL

/*! \file */
///@{
/*!
 * \fn std::unique_ptr<ProjectorExtensionType> CTL::makeExtension(AbstractProjector* projector = nullptr)
 * \relates ProjectorExtension
 * Global (free) make function that creates a new ProjectorExtension by taking over the ownership of
 * a \a projector. The \a projector will be the nested projector of the returned ProjectorExtension.
 * The component is returned as a `std::unique_ptr<ProjectorExtensionType>`, whereas
 * `ProjectorExtensionType` is the template argument of this function that needs to be specified.
 */

/*!
 * \fn std::unique_ptr<ProjectorExtensionType> CTL::makeExtension(std::unique_ptr<AbstractProjector> projector)
 * \relates ProjectorExtension
 * Global (free) make function that creates a new ProjectorExtension by taking over the ownership of
 * a \a projector. The \a projector will be the nested projector of the returned ProjectorExtension.
 * The component is returned as a `std::unique_ptr<ProjectorExtensionType>`, whereas
 * `ProjectorExtensionType` is the template argument of this function that needs to be specified.
 */
///@}

#endif // PROJECTOREXTENSION_H
