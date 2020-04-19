#ifndef CTL_PROJECTOREXTENSION_H
#define CTL_PROJECTOREXTENSION_H

#include "abstractprojector.h"
#include <memory>

namespace CTL {
/*!
 * \class ProjectorExtension
 *
 * \brief The ProjectorExtension class provides the interface to extend projectors with additional
 * functionality using the concept of decoration.
 *
 * The ProjectorExtension class allowes to extend the functionalty of any projector, i.e. of any
 * class derived from AbstractProjector. The idea is that an extension *uses* another projector as
 * a nested projector that is called. Before and after that call any modification can be made.
 * The nested projector itself may also be an extension.
 * Note that the nested projector is *owned* by the extension meaning that when the extension gets
 * destroyed the nested projector will be destroyed too.
 *
 * There are several syntactical ways to extend another projector/extension.
 * The following snippets show exemplarily the PoissonNoiseExtension extending the
 * OCL::RayCasterProjector. Each pattern supports raw pointers as well as `std::unique_ptr`s.
 * 1. By using the contructor
 * \code
 * // with `new` and raw pointers
 * auto extRaw  = new PoissonNoiseExtension(new OCL::RayCasterProjector);
 *
 * // with make functions and `std::unique_ptr`s (safer in terms of accidental memory leaks)
 * auto extUPtr = makeExtension<PoissonNoiseExtension>(makeProjector<OCL::RayCasterProjector>());
 * \endcode
 * 2. By using the ProjectorExtension::use method
 * \code
 * auto ext = makeExtension<PoissonNoiseExtension>();
 * ext->use(makeProjector<OCL::RayCasterProjector>());
 * \endcode
 * 3. By using pipe operators (pay attention to the order)
 * \code
 * auto ext1 = makeProjector<OCL::RayCasterProjector>() |
 *             makeExtension<PoissonNoiseExtension>();
 *
 * // or in separate steps
 * std::unique_ptr<AbstractProjector> ext2;
 * ext2 = makeProjector<OCL::RayCasterProjector>();
 * ext2 |= makeExtension<PoissonNoiseExtension>();
 *
 * // also mixtures of unique and raw pointers are possible
 * auto ext3 = makeProjector<OCL::RayCasterProjector>() | new PoissonNoiseExtension;
 * \endcode
 * 4. (3b) By using the `pipe` function for two raw pointers
 * (because two raw pointers cannot be combined via '`|`' or '`|=`')
 * \code
 * AbstractProjector* ext;
 * ext = new OCL::RayCasterProjector;
 * pipe(ext, new PoissonNoiseExtension);
 *
 * // multiple concatenation is also possible similar to the pipe operators
 * pipe(pipe(ext, new PoissonNoiseExtension), new ProjectorExtension);
 * // Note: '... | ProjectorExtension' does not change the projector's behavior
 * \endcode
 *
 * In order to build and manage a larger pipeline of `ProjectorExtension`s, see the helper class
 * ProjectionPipeline. A pre-defined (suggested) pipeline is provided by the StandardPipeline class.
 *
 * When implementing a custom extension, your class must be a subclass of ProjectorExtension.
 * There are two possiblilties to implement such an extension:
 * 1. Override `extendedProject`
 * 2. Override `project` and `projectComposite`.
 *
 * Doing 1. and 2. at the same time is not sensible as the implementation of `extendedProject` will
 * have no effect.
 *
 * You would like to override the AbstractProjector::configure method as well in order to initialize
 * your Extension with informations from the AcquisitionSetup. When doing so, you should also call
 * ProjectorExtension::configure in order to configure the nested projector.
 *
 * If your extension \f$ E \f$ leads to a non-linear projector (regarding `project` or
 * `projectComposite`), i.e.
 * \f$
 * E(a\boldsymbol{v_{1}}+\boldsymbol{v_{2}})\neq aE\boldsymbol{v_{1}}+E\boldsymbol{v_{2}}
 * \f$
 * , with volumes \f$ \boldsymbol{v_{1}} \f$, \f$ \boldsymbol{v_{2}} \f$ and \f$ a\in \mathbb{R} \f$,
 * then you should override AbstractProjector::isLinear so that it returns false. Otherwise do not
 * override this method---in particular do not override with `true`.
 *
 * The full effect of an extension can be formalized as follows. An extension \f$ E \f$ can be
 * decomposed into three operators:
 *
 * \f$
 * E\left\{A_{\boldsymbol{\pi}}\right\}\boldsymbol{v}=P\,A_{\Pi\boldsymbol{\pi}}\,V\boldsymbol{v},
 * \f$
 *
 * a modficication \f$ V \f$ of the `VolumeData` \f$ \boldsymbol{v} \f$,
 * a modification \f$ \Pi \f$ of the AcquisitionSetup \f$ \boldsymbol{\pi} \f$ and
 * a modification \f$ P \f$ of the ProjectionData after projection with the nested projector
 * \f$ A_{\Pi\boldsymbol{\pi}} \f$ (that uses the modified AcquisitionSetup
 * \f$ \Pi\boldsymbol{\pi} \f$).
 * All three operators "maintain the dimensionality" of the objects they affect. Only the nested
 * projector \f$ A \f$ maps from volume space to projection space. In case you directly use the
 * ProjectorExtension class (without any customization) all three operators are identity maps, i.e.
 * it reduces to
 *
 * \f$
 * E\left\{A_{\boldsymbol{\pi}}\right\}\boldsymbol{v}=A_{\boldsymbol{\pi}}\,\boldsymbol{v}\,,
 * \f$
 *
 * meaning that only the nested projector is applied.
 *
 * In case you opt for variant 1, i.e. you override the `extendedProject` method, you can only
 * implement the \f$ P \f$ operator, i.e.
 *
 * \f$
 * E\left\{A_{\boldsymbol{\pi}}\right\}\boldsymbol{v}=PA_{\boldsymbol{\pi}}\,\boldsymbol{v}\,.
 * \f$
 *
 * This would lead to an extension that perform only post-processing of the ProjectionData, i.e.
 * it represents a part in a classical pipeline.
 * The variant 2 effectively enables the implementation of all the three operators
 * \f$ V \f$, \f$ \Pi \f$ and \f$ P \f$.
 *
 * When considering a concatenation of two extensions, first \f$ E_1 \f$ and then \f$ E_2 \f$, the
 * following order of operators is applied by design:
 *
 * \f$
 * \left(E_{2}\circ E_{1}\right)\left\{ A_{\boldsymbol{\pi}}\right\} \boldsymbol{v}=
 * P_{2}P_{1}\,A_{\Pi_{1}\Pi_{2}\boldsymbol{\boldsymbol{\pi}}}\,V_{1}V_{2}\boldsymbol{v}.
 * \f$
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

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;
    public: ProjectionData project(const VolumeData& volume) override;

public:
    explicit ProjectorExtension(AbstractProjector* projector = nullptr);
    explicit ProjectorExtension(std::unique_ptr<AbstractProjector> projector);

    ~ProjectorExtension() override;

    // virtual methods
    ProjectionData projectComposite(const CompositeVolume& volume) override;
    bool isLinear() const override;
    virtual void use(AbstractProjector* other);
    void use(std::unique_ptr<AbstractProjector> other);

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

#endif // CTL_PROJECTOREXTENSION_H
