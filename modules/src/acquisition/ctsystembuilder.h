#ifndef CTL_CTSYSTEMBUILDER_H
#define CTL_CTSYSTEMBUILDER_H

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
    virtual ~AbstractCTsystemBlueprint() = default;

    // optionally reimplementable
    virtual QString systemName() const { return QStringLiteral("Blueprinted system"); }
    virtual std::vector<GenericBeamModifier*> modifiers() const { return { }; }

protected:
    AbstractCTsystemBlueprint() = default;
    AbstractCTsystemBlueprint(const AbstractCTsystemBlueprint&) = default;
    AbstractCTsystemBlueprint(AbstractCTsystemBlueprint&&) = default;
    AbstractCTsystemBlueprint& operator=(const AbstractCTsystemBlueprint&) = default;
    AbstractCTsystemBlueprint& operator=(AbstractCTsystemBlueprint&&) = default;
};

class CTsystemBuilder
{
public:
    static CTsystem createFromBlueprint(const AbstractCTsystemBlueprint& systemBlueprint);
    static CTsystem createFromJSONFile(const QString& fileName);
};

} // namespace CTL

#endif // CTL_CTSYSTEMBUILDER_H
