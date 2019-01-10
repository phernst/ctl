#ifndef CTSYSTEMBUILDER_H
#define CTSYSTEMBUILDER_H

#include "components/allgenerictypes.h"
#include "acquisition/ctsystem.h"

namespace CTL {

class AbstractCTsystemBlueprint
{
    // abstract interface (must-haves of a complete CTsystem)
    public:virtual AbstractDetector* detector() const = 0;
    public:virtual AbstractGantry* gantry() const = 0;
    public:virtual AbstractSource* source() const = 0;

public:
    AbstractCTsystemBlueprint() = default;

    // default copy/move behavior with virtual destructor
    AbstractCTsystemBlueprint(const AbstractCTsystemBlueprint&) = default;
    AbstractCTsystemBlueprint(AbstractCTsystemBlueprint&&) = default;
    AbstractCTsystemBlueprint& operator=(const AbstractCTsystemBlueprint&) = default;
    AbstractCTsystemBlueprint& operator=(AbstractCTsystemBlueprint&&) = default;
    virtual ~AbstractCTsystemBlueprint() = default;

    // optionally reimplementable
    virtual QString systemName() const { return QStringLiteral("Blueprinted system"); }
    virtual std::vector<GenericBeamModifier*> modifiers() const { return {}; }
};

class CTsystemBuilder
{
public:
    static CTsystem createFromBlueprint(const AbstractCTsystemBlueprint& systemBlueprint);
    static CTsystem createFromJSON(const QJsonObject& json);
};

} // namespace CTL

#endif // CTSYSTEMBUILDER_H
