#ifndef CTL_CTSYSTEMBUILDER_H
#define CTL_CTSYSTEMBUILDER_H

#include "components/allgenerictypes.h"
#include "acquisition/ctsystem.h"

namespace CTL {

class AbstractCTSystemBlueprint
{
    // abstract interface (must-haves of a complete CTSystem)
    public:virtual AbstractDetector* detector() const = 0;
    public:virtual AbstractGantry* gantry() const = 0;
    public:virtual AbstractSource* source() const = 0;

public:
    virtual ~AbstractCTSystemBlueprint() = default;

    // optionally reimplementable
    virtual QString systemName() const { return QStringLiteral("Blueprinted system"); }
    virtual std::vector<GenericBeamModifier*> modifiers() const { return { }; }

protected:
    AbstractCTSystemBlueprint() = default;
    AbstractCTSystemBlueprint(const AbstractCTSystemBlueprint&) = default;
    AbstractCTSystemBlueprint(AbstractCTSystemBlueprint&&) = default;
    AbstractCTSystemBlueprint& operator=(const AbstractCTSystemBlueprint&) = default;
    AbstractCTSystemBlueprint& operator=(AbstractCTSystemBlueprint&&) = default;
};

class CTSystemBuilder
{
public:
    static CTSystem createFromBlueprint(const AbstractCTSystemBlueprint& systemBlueprint);
    static CTSystem createFromJSONFile(const QString& fileName);
};

} // namespace CTL

#endif // CTL_CTSYSTEMBUILDER_H
