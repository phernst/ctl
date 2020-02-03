#ifndef ABSTRACTPROJECTORCONFIG_H
#define ABSTRACTPROJECTORCONFIG_H

namespace CTL {

/*!
 * \class AbstractProjectorConfig
 *
 * \brief The AbstractProjectorConfig class is the abstract base class for objects that hold the
 * configuration of a projector.
 *
 * This class should be sub-classed and extended by all configuration information (e.g. parameters)
 * required by the projector it shall be used with.
 */
class AbstractProjectorConfig
{
    // abstract interface
    public:virtual AbstractProjectorConfig* clone() const = 0;

public:
    virtual ~AbstractProjectorConfig() = default;

protected:
    AbstractProjectorConfig() = default;
    AbstractProjectorConfig(const AbstractProjectorConfig&) = default;
    AbstractProjectorConfig(AbstractProjectorConfig&&) = default;
    AbstractProjectorConfig& operator=(const AbstractProjectorConfig&) = default;
    AbstractProjectorConfig& operator=(AbstractProjectorConfig&&) = default;
};

/*!
 * \fn AbstractProjectorConfig::AbstractProjectorConfig()
 *
 * Default constructor.
 */

/*!
 * \fn AbstractProjectorConfig::AbstractProjectorConfig(const AbstractProjectorConfig&)
 *
 * Default copy constructor.
 */

/*!
 * \fn AbstractProjectorConfig::AbstractProjectorConfig(AbstractProjectorConfig&&)
 *
 * Default move constructor.
 */

/*!
 * \fn AbstractProjectorConfig& AbstractProjectorConfig::operator= (const AbstractProjectorConfig&)
 *
 * Default assignment operator.
 */

/*!
 * \fn AbstractProjectorConfig& AbstractProjectorConfig::operator= (AbstractProjectorConfig&&)
 *
 * Default move-assignment operator.
 */

/*!
 * \fn AbstractProjectorConfig::~AbstractProjectorConfig()
 *
 * Default destructor.
 */

/*!
 * \fn AbstractProjectorConfig* AbstractProjectorConfig::clone()
 *
 * Creates a copy of this instance and returns a base class pointer to the new object.
 */

} // namespace CTL

/*! \file */

#endif // ABSTRACTPROJECTORCONFIG_H
