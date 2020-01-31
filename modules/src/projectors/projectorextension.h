#ifndef PROJECTOREXTENSION_H
#define PROJECTOREXTENSION_H

#include "abstractprojector.h"
#include <memory>
#include <QDebug>

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
    CTL_TYPE_ID(100)

protected:
    class MetaProjector
    {
    public:
        MetaProjector(const VolumeData& volume, AbstractProjector* projector);
        MetaProjector(const CompositeVolume& volume, AbstractProjector* projector);

        ProjectionData project() const;
        bool isComposite() const;

    private:
        AbstractProjector* _projector;
        const VolumeData* _simpleVolume;
        const CompositeVolume* _compositeVolume;
    };


public:
    explicit ProjectorExtension(AbstractProjector* projector = nullptr);
    explicit ProjectorExtension(std::unique_ptr<AbstractProjector> projector);

    ~ProjectorExtension() override;

    // virtual methods
    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;
    ProjectionData projectComposite(const CompositeVolume& volume) override;
    bool isLinear() const override;
    virtual void use(AbstractProjector* other);
    virtual void use(std::unique_ptr<AbstractProjector> other);

    // SerializationInterface interface
    void fromVariant(const QVariant &variant) override;
    QVariant toVariant() const override;

    // other methods
    AbstractProjector* release();
    void reset();

protected:
    virtual ProjectionData extendedProject(const MetaProjector& nestedProjector);

private:
    AbstractProjector* _projector; //!< The nested projector object.
};


// pipe operators
// u_ptr, u_ptr
template<class ProjectorExtensionType>
auto operator|(std::unique_ptr<AbstractProjector> lhs,
               std::unique_ptr<ProjectorExtensionType> rhs) ->
typename std::enable_if<std::is_convertible<ProjectorExtensionType*, ProjectorExtension*>::value,
                        std::unique_ptr<ProjectorExtensionType>>::type
{
    rhs->use(std::move(lhs));
    return rhs;
}

// u_ptr, raw_ptr
template<class ProjectorExtensionType>
auto operator|(std::unique_ptr<AbstractProjector> lhs,
               ProjectorExtensionType* rhs) ->
typename std::enable_if<std::is_convertible<ProjectorExtensionType*, ProjectorExtension*>::value,
                        std::unique_ptr<ProjectorExtensionType>>::type
{
    rhs->use(std::move(lhs));
    return std::unique_ptr<ProjectorExtensionType>{ rhs };
}

// raw_ptr, u_ptr
template<class ProjectorExtensionType>
auto operator|(AbstractProjector* lhs,
               std::unique_ptr<ProjectorExtensionType> rhs) ->
typename std::enable_if<std::is_convertible<ProjectorExtensionType*, ProjectorExtension*>::value,
                        std::unique_ptr<ProjectorExtensionType>>::type
{
    rhs->use(lhs);
    return rhs;
}

// u_ptr, u_ptr
std::unique_ptr<AbstractProjector>& operator|=(std::unique_ptr<AbstractProjector>& lhs,
                                                      std::unique_ptr<ProjectorExtension> rhs);
std::unique_ptr<ProjectorExtension>& operator|=(std::unique_ptr<ProjectorExtension>& lhs,
                                                      std::unique_ptr<ProjectorExtension> rhs);
// u_ptr, raw_ptr
std::unique_ptr<AbstractProjector>& operator|=(std::unique_ptr<AbstractProjector>& lhs,
                                                      ProjectorExtension* rhs);
std::unique_ptr<ProjectorExtension>& operator|=(std::unique_ptr<ProjectorExtension>& lhs,
                                                      ProjectorExtension* rhs);
// raw_ptr, u_ptr
AbstractProjector*& operator|=(AbstractProjector*& lhs,
                                      std::unique_ptr<ProjectorExtension> rhs);
ProjectorExtension*& operator|=(ProjectorExtension*& lhs,
                                      std::unique_ptr<ProjectorExtension> rhs);
// raw_ptr, raw_ptr
AbstractProjector*& pipe(AbstractProjector*& lhs, ProjectorExtension* rhs);
ProjectorExtension*& pipe(ProjectorExtension*& lhs, ProjectorExtension* rhs);


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
