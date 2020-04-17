#include "simplectsystem.h"
#include "acquisition/geometryencoder.h"
#include "acquisition/radiationencoder.h"
#include "components/allgenerictypes.h"

namespace CTL {

/*!
 * \fn SimpleCTSystem::SimpleCTSystem()
 *
 * Default constructor. In protected domain and not intended for external use.
 */

/*!
 * Constructs a SimpleCTSystem with the components \a detector, \a gantry and \a source.
 */
SimpleCTSystem::SimpleCTSystem(const AbstractDetector& detector,
                               const AbstractGantry& gantry,
                               const AbstractSource& source)
{
    *this << detector.clone() << gantry.clone() << source.clone();
}

/*!
 * Constructs a SimpleCTSystem with the components \a detector, \a gantry and \a source.
 */
SimpleCTSystem::SimpleCTSystem(AbstractDetector&& detector,
                               AbstractGantry&& gantry,
                               AbstractSource&& source)
{
    *this << std::move(detector).clone() << std::move(gantry).clone() << std::move(source).clone();
}

/*!
 * Constructs a SimpleCTSystem object as a copy of the CTSystem \a system.
 *
 * Note that this constructor does not check whether \a system can properly be "converted"
 * into a SimpleCTSystem. This is acceptable here, since it is protected  and is only called
 * from the public factory method fromCTsystem(), which already makes sure that the system is
 * simple (and thus, can be converted).
 */
SimpleCTSystem::SimpleCTSystem(const CTSystem& system)
    : CTSystem(system)
{
}

/*!
 * Move-constructs a SimpleCTSystem object from CTSystem \a system.
 *
 * Note that this constructor does not check whether \a system can properly be "converted"
 * into a SimpleCTSystem. This is acceptable here, since it is protected and is only called
 * from the public factory method fromCTsystem(), which already makes sure that the system is
 * simple (and thus, can be converted).
 */
SimpleCTSystem::SimpleCTSystem(CTSystem&& system)
    : CTSystem(std::move(system))
{
}

/*!
 * Constructs and returns a SimpleCTSystem object from the CTSystem \a system. Returns an empty
 * object if \a system is not simple (isEmpty() of the returned object will return true).
 * If \a ok is nonnull \a *ok will be set to true if the conversion to a SimpleCTSystem was
 * successful, i.e. \a system is simple, otherwise \a *ok will be set to false.
 * If \a ok is a nullptr and the \a system is not simple, an exception will be thrown.
 */
SimpleCTSystem SimpleCTSystem::fromCTsystem(const CTSystem& system, bool* ok)
{
    // check if 'system' has a simple configuration (i.e. only one source, detector, and gantry)
    if(system.isSimple())
    {
        if(ok)
            *ok = true;
        return { system };
    }
    else
    {
        if(ok)
            *ok = false;
        else
            throw std::runtime_error("SimpleCTSystem::fromCTsystem(const CTSystem&): "
                                     "System is not simple.");
        return { };
    }
}

/*!
 * Overload of SimpleCTSystem::fromCTsystem(const CTSystem& system, bool* ok) that binds to a
 * \a system passed as a rvalue.
 * An exception will be thrown if \a system is not simple and \a ok is a nullptr.
 */
SimpleCTSystem SimpleCTSystem::fromCTsystem(CTSystem&& system, bool* ok)
{
    // check if 'system' has a simple configuration (i.e. only one source, detector, and gantry)
    if(system.isSimple())
    {
        if(ok)
            *ok = true;
        return { std::move(system) };
    }
    else
    {
        if(ok)
            *ok = false;
        else
            throw std::runtime_error("SimpleCTSystem::fromCTsystem(CTSystem&&): "
                                     "System is not simple.");
        return { };
    }
}

/*!
 * Returns a pointer to the detector component in the system. This does not transfer ownership.
 */
AbstractDetector* SimpleCTSystem::detector() const
{
    auto dtctrs = detectors();
    Q_ASSERT(!dtctrs.empty());
    return dtctrs.empty() ? nullptr : dtctrs.front();
}

/*!
 * Returns a pointer to the gantry component in the system. This does not transfer ownership.
 */
AbstractGantry* SimpleCTSystem::gantry() const
{
    auto gntrs = gantries();
    Q_ASSERT(!gntrs.empty());
    return gntrs.empty() ? nullptr : gntrs.front();
}

/*!
 * Returns a pointer to the source component in the system. This does not transfer ownership.
 */
AbstractSource* SimpleCTSystem::source() const
{
    auto srcs = sources();
    Q_ASSERT(!srcs.empty());
    return srcs.empty() ? nullptr : srcs.front();
}

/*!
 * Replaces the detector component of this instance by \a newDetector. The old detector object is
 * destroyed. This instance takes ownership of \a newDetector.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTSystem::replaceDetector(AbstractDetector* newDetector)
{
    Q_ASSERT(newDetector);
    if(newDetector)
    {
        removeComponent(detector());
        addComponent(newDetector);
    }
}

/*!
 * Replaces the detector component of this instance by \a newDetector. The old detector object is
 * destroyed. This instance takes ownership of \a newDetector.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTSystem::replaceDetector(std::unique_ptr<AbstractDetector> newDetector)
{
    replaceDetector(newDetector.release());
}

/*!
 * Replaces the gantry component of this instance by \a newGantry. The old gantry object is
 * destroyed. This instance takes ownership of \a newGantry.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTSystem::replaceGantry(AbstractGantry* newGantry)
{
    Q_ASSERT(newGantry);
    if(newGantry)
    {
        removeComponent(gantry());
        addComponent(newGantry);
    }
}

/*!
 * Replaces the gantry component of this instance by \a newGantry. The old gantry object is
 * destroyed. This instance takes ownership of \a newGantry.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTSystem::replaceGantry(std::unique_ptr<AbstractGantry> newGantry)
{
    replaceGantry(newGantry.release());
}

/*!
 * Replaces the source component of this instance by \a newSource. The old source object is
 * destroyed. This instance takes ownership of \a newSource.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTSystem::replaceSource(AbstractSource* newSource)
{
    Q_ASSERT(newSource);
    if(newSource)
    {
        removeComponent(source());
        addComponent(newSource);
    }
}

/*!
 * Replaces the source component of this instance by \a newSource. The old source object is
 * destroyed. This instance takes ownership of \a newSource.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTSystem::replaceSource(std::unique_ptr<AbstractSource> newSource)
{
    replaceSource(newSource.release());
}

/*!
 * Adds the AbstractBeamModifier \a modifier to the system. This instance takes ownership of
 * \a modifier.
 */
void SimpleCTSystem::addBeamModifier(AbstractBeamModifier* modifier) { addComponent(modifier); }

/*!
 * Adds the AbstractBeamModifier \a modifier to the system.
 */
void SimpleCTSystem::addBeamModifier(std::unique_ptr<AbstractBeamModifier> modifier)
{
    addComponent(std::move(modifier));
}

/*!
 * Returns the number of photons that incide on a detector pixel averaged over all detector
 * modules.
 */
float SimpleCTSystem::photonsPerPixelMean() const
{
    return RadiationEncoder(this).photonsPerPixelMean();
}

/*!
 * Returns the average number of photons that incide on a detector pixel in module \a module.
 */
float SimpleCTSystem::photonsPerPixel(uint module) const
{
    return RadiationEncoder(this).photonsPerPixel(module);
}

/*!
 * Returns the average numbers of photons that incide on a detector pixel for all modules.
 */
std::vector<float> SimpleCTSystem::photonsPerPixel() const
{
    return RadiationEncoder(this).photonsPerPixel();
}

// use documentation of CTSystem::clone()
CTSystem* SimpleCTSystem::clone() const & { return new SimpleCTSystem(*this); }

// use documentation of CTSystem::clone()
CTSystem* SimpleCTSystem::clone() && { return new SimpleCTSystem(std::move(*this)); }

} // namespace CTL
