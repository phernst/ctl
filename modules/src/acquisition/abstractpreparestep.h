#ifndef ABSTRACTPREPARESTEP_H
#define ABSTRACTPREPARESTEP_H

#include <memory>
#include <vector>
#include <QDebug>

#include "io/serializationinterface.h"

typedef unsigned int uint;

/*
 * NOTE: This is header only.
 */

namespace CTL {

class AcquisitionSetup;
class CTsystem;
class SimpleCTsystem;

/*!
 * \class AbstractPrepareStep
 *
 * \brief Base class for preparation steps used within an AcquisitionSetup.
 *
 * Preparation steps are used to bring certain components in the desired state for an upcoming image
 * acquisition. This usually sets member variables to defined values.
 *
 * Sub-classing of AbstractPrepareStep is recommended for each sub-class of SystemComponent to allow
 * for the preparation of their state in an AcquisitionSetup. Any sub-class must implement the
 * abstract interface methods prepare() and isApplicableTo().
 *
 * The isApplicableTo() method shall serve as an option to verify whether a specific prepare step
 * can be applied to a given CTsystem. This usually checks if the required components (which are
 * to be prepared) are present in the system.
 *
 * In the prepare() method, the actual preparation of the system state is performed. Note that this
 * is not necessarily limited to changes to a single component within the system.
 */
class AbstractPrepareStep : public SerializationInterface
{
    // abstract interface
    public:virtual void prepare(SimpleCTsystem* system) const = 0;
    public:virtual bool isApplicableTo(const CTsystem& system) const = 0;

public:
    enum { Type = 0, UserType = 65536 };

    virtual int type() const;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    virtual ~AbstractPrepareStep() = default;
};

/*!
 * \class AbstractPreparationProtocol
 *
 * \brief Base class for entire preparation protocols (i.e. for multiple views) that can be used in
 * combination with an AcquisitionSetup.
 *
 * A preparation protocol holds the information about all necessary preparation steps for each view
 * in an entire acquisition. Its abstract interface prepareSteps() can be interpreted as a factory
 * method that constructs all prepare steps required for the preparation of a certain view in the
 * desired protocol.
 *
 * For convenience, AbstractPreparationProtocol can also provide a method to confirm its
 * applicability to a given AcquisitionSetup - just as AbstractPrepareStep does for a given
 * CTsystem. This should be reimplemented in sub-classes to cover all dependencies of the particular
 * protocol.
 */
class AbstractPreparationProtocol
{
    // abstract interface
    public:virtual std::vector<std::shared_ptr<AbstractPrepareStep>>
                   prepareSteps(uint viewNb, const AcquisitionSetup& setup) const = 0;

public:
    virtual bool isApplicableTo(const AcquisitionSetup& setup) const;
    virtual ~AbstractPreparationProtocol() = default;
};

// placeholder, implement meaningfully in sub-classes
inline bool AbstractPreparationProtocol::isApplicableTo(const AcquisitionSetup&) const
{
    return true;
}

/*!
 * \fn AbstractPrepareStep::prepare(SimpleCTsystem* system) const
 *
 * This method performs the actual preparation of the state of \a system. This usually consists of
 * setting member variables of certain components in \a system to defined values.
 *
 * Note that this does not necessarily have to be limited to changes to a single component within
 * the system.
 */

/*!
 * \fn AbstractPrepareStep::isApplicableTo(const CTsystem& system) const
 *
 * Returns true if this prepare step can be applied to \a system.
 *
 * Typically, this method will check whether \a system contains all components that shall be
 * prepared by this instance.
 */

/*!
 * \fn AbstractPrepareStep::~AbstractPrepareStep()
 *
 * Default (virtual) destructor.
 */

/*!
 * \fn AbstractPreparationProtocol::prepareSteps(uint viewNb, const AcquisitionSetup& setup) const
 *
 * Returns a vector containing all preparation steps required to prepare the system in \a setup for
 * acquisition of view \a viewNb.
 */

/*!
 * \fn AbstractPreparationProtocol::isApplicableTo(const AcquisitionSetup& setup) const
 *
 * Returns true if this protocol can be used with \a setup.
 *
 * Typically, this method will check whether the system used in \a setup contains all components
 * that shall be prepared by this instance and whether the available information in this instance is
 * compatible with the number of views specified in \a setup.
 */

/*!
 * \fn AbstractPreparationProtocol::~AbstractPreparationProtocol()
 *
 * Default (virtual) destructor.
 */

/*!
 * Returns the type id of the prepare step.
 *
 * List of all default types:
 *
 * Type                          | Type-ID
 * ------------------------------|--------------
 * AbstractPrepareStep::Type     |   0
 * GenericDetectorParam::Type    | 101
 * GenericGantryParam::Type      | 201
 * CarmGantryParam::Type         | 210
 * TubularGantryParam::Type      | 220
 * GantryDisplacementParam::Type | 230
 * SourceParam::Type             | 300
 * XrayLaserParam::Type          | 310
 * XrayTubeParam::Type           | 320
 */
inline int AbstractPrepareStep::type() const { return Type; }

inline void AbstractPrepareStep::fromVariant(const QVariant &variant)
{
    auto varMap = variant.toMap();
    if(varMap.value("type-id").toInt() != type())
    {
        qWarning() << QString(typeid(*this).name())
                + "::fromVariant: Could not construct instance! "
                  "reason: incompatible variant passed";
        return;
    }
}

inline QVariant AbstractPrepareStep::toVariant() const
{
    QVariantMap ret;

    ret.insert("type-id", type());
    ret.insert("name", typeid(*this).name());

    return ret;
}

/*!
 * \def ADD_TO_PREPARESTEP_ENUM(newIndex)
 *
 * Macro to add the component to the component enumeration with the index \a newIndex.
 */
#define ADD_TO_PREPARESTEP_ENUM(newIndex)                                                          \
public:                                                                                            \
    enum { Type = newIndex };                                                                      \
    int type() const override { return Type; }                                                     \
                                                                                                   \
private:                                                                                           \
    template<class>                                                                                \
    friend struct SerializationInterface::RegisterWithJsonSerializer;
} // namespace CTL

#endif // ABSTRACTPREPARESTEP_H
