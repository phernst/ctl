#include "simplectsystem.h"
#include "acquisition/geometryencoder.h"
#include "acquisition/radiationencoder.h"
#include "components/allgenerictypes.h"

namespace CTL {

/*!
 * \fn SimpleCTsystem::SimpleCTsystem()
 *
 * Default constructor. In protected domain and not intended for external use.
 */

/*!
 * Constructs a SimpleCTsystem with the components \a detector, \a gantry and \a source.
 */
SimpleCTsystem::SimpleCTsystem(const AbstractDetector& detector,
                               const AbstractGantry& gantry,
                               const AbstractSource& source)
{
    *this << detector.clone() << gantry.clone() << source.clone();
}

/*!
 * Constructs a SimpleCTsystem with the components \a detector, \a gantry and \a source.
 */
SimpleCTsystem::SimpleCTsystem(AbstractDetector&& detector,
                               AbstractGantry&& gantry,
                               AbstractSource&& source)
{
    *this << std::move(detector).clone() << std::move(gantry).clone() << std::move(source).clone();
}

/*!
 * Constructs a SimpleCTsystem object as a copy of the CTsystem \a system.
 *
 * Note that this constructor does not check whether \a system can properly be "converted"
 * into a SimpleCTsystem. This is acceptable here, since it is protected  and is only called
 * from the public factory method fromCTsystem(), which already makes sure that the system is
 * simple (and thus, can be converted).
 */
SimpleCTsystem::SimpleCTsystem(const CTsystem& system)
    : CTsystem(system)
{
}

/*!
 * Move-constructs a SimpleCTsystem object from CTsystem \a system.
 *
 * Note that this constructor does not check whether \a system can properly be "converted"
 * into a SimpleCTsystem. This is acceptable here, since it is protected and is only called
 * from the public factory method fromCTsystem(), which already makes sure that the system is
 * simple (and thus, can be converted).
 */
SimpleCTsystem::SimpleCTsystem(CTsystem&& system)
    : CTsystem(std::move(system))
{
}

/*!
 * Constructs and returns a SimpleCTsystem object from the CTsystem \a system. Returns an empty
 * object if \a system is not simple (isEmpty() of the returned object will return true).
 * If \a ok is nonnull \a *ok will be set to true if the conversion to a SimpleCTsystem was
 * successful, i.e. \a system is simple, otherwise \a *ok will be set to false.
 */
SimpleCTsystem SimpleCTsystem::fromCTsystem(const CTsystem& system, bool* ok)
{
    // check if 'system' has a simple configuration (i.e. only one source, detector, and gantry)
    if(system.isSimple())
    {
        if(ok) *ok = true;
        return SimpleCTsystem(system);
    }
    else
    {
        if(ok) *ok = false;
        return SimpleCTsystem();
    }
}

/*!
 * Overload of SimpleCTsystem::fromCTsystem(const CTsystem& system, bool* ok) that binds to a
 * \a system passed as a rvalue.
 */
SimpleCTsystem SimpleCTsystem::fromCTsystem(CTsystem&& system, bool* ok)
{
    // check if 'system' has a simple configuration (i.e. only one source, detector, and gantry)
    if(system.isSimple())
    {
        if(ok) *ok = true;
        return SimpleCTsystem(std::move(system));
    }
    else
    {
        if(ok) *ok = false;
        return SimpleCTsystem();
    }
}

/*!
 * Returns a pointer to the detector component in the system. This does not transfer ownership.
 */
AbstractDetector* SimpleCTsystem::detector() const
{
    auto dtctrs = detectors();
    Q_ASSERT(!dtctrs.empty());
    return dtctrs.empty() ? nullptr : dtctrs.front();
}

/*!
 * Returns a pointer to the gantry component in the system. This does not transfer ownership.
 */
AbstractGantry* SimpleCTsystem::gantry() const
{
    auto gntrs = gantries();
    Q_ASSERT(!gntrs.empty());
    return gntrs.empty() ? nullptr : gntrs.front();
}

/*!
 * Returns a pointer to the source component in the system. This does not transfer ownership.
 */
AbstractSource* SimpleCTsystem::source() const
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
void SimpleCTsystem::replaceDetector(AbstractDetector* newDetector)
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
void SimpleCTsystem::replaceDetector(std::unique_ptr<AbstractDetector> newDetector)
{
    replaceDetector(newDetector.release());
}

/*!
 * Replaces the gantry component of this instance by \a newGantry. The old gantry object is
 * destroyed. This instance takes ownership of \a newGantry.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTsystem::replaceGantry(AbstractGantry* newGantry)
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
void SimpleCTsystem::replaceGantry(std::unique_ptr<AbstractGantry> newGantry)
{
    replaceGantry(newGantry.release());
}

/*!
 * Replaces the source component of this instance by \a newSource. The old source object is
 * destroyed. This instance takes ownership of \a newSource.
 *
 * Does nothing if a nullptr is passed.
 */
void SimpleCTsystem::replaceSource(AbstractSource* newSource)
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
void SimpleCTsystem::replaceSource(std::unique_ptr<AbstractSource> newSource)
{
    replaceSource(newSource.release());
}

/*!
 * Adds the AbstractBeamModifier \a modifier to the system. This instance takes ownership of
 * \a modifier.
 */
void SimpleCTsystem::addBeamModifier(AbstractBeamModifier* modifier) { addComponent(modifier); }

/*!
 * Adds the AbstractBeamModifier \a modifier to the system.
 */
void SimpleCTsystem::addBeamModifier(std::unique_ptr<AbstractBeamModifier> modifier)
{
    addComponent(std::move(modifier));
}

/*!
 * Returns the number of photons that incide on a detector pixel averaged over all detector
 * modules.
 */
float SimpleCTsystem::photonsPerPixelMean() const
{
    return RadiationEncoder(this).photonsPerPixelMean();
}

/*!
 * Returns the average number of photons that incide on a detector pixel in module \a module.
 */
float SimpleCTsystem::photonsPerPixel(uint module) const
{
    return RadiationEncoder(this).photonsPerPixel(module);
}

/*!
 * Returns the average numbers of photons that incide on a detector pixel for all modules.
 */
std::vector<float> SimpleCTsystem::photonsPerPixel() const
{
    return RadiationEncoder(this).photonsPerPixel();
}

// use documentation of CTsystem::clone()
CTsystem* SimpleCTsystem::clone() const & { return new SimpleCTsystem(*this); }

// use documentation of CTsystem::clone()
CTsystem* SimpleCTsystem::clone() && { return new SimpleCTsystem(std::move(*this)); }

} // namespace CTL
