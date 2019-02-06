#ifndef ACQUISITIONSETUP_H
#define ACQUISITIONSETUP_H

#include "abstractpreparestep.h"
#include "simplectsystem.h"

namespace CTL {

/*!
 * \class AcquisitionSetup
 *
 * \brief Holds a CTsystem together with the information about the system settings for all views
 * from which projection images shall be simulated.
 *
 *
 */
class AcquisitionSetup final : public SerializationInterface
{
public:
    typedef std::shared_ptr<const AbstractPrepareStep> PrepareStep;

    class View : public SerializationInterface
    {
    public:
        View() = default;
        View(double time);

        void setTimeStamp(double timeStamp);
        void addPrepareStep(PrepareStep step);

        double timeStamp() const;
        const std::vector<PrepareStep>& prepareSteps() const;

        void clearPrepareSteps();

        void fromVariant(const QVariant& variant) override; // de-serialization
        QVariant toVariant() const override; // serialization

    private:
        double _timeStamp;
        std::vector<PrepareStep> _prepareSteps;
    };

    AcquisitionSetup() = default;
    AcquisitionSetup(const CTsystem& system, uint nbViews = 0);
    AcquisitionSetup(CTsystem&& system, uint nbViews = 0);

    // cp/mv cstor/assignment
    AcquisitionSetup(const AcquisitionSetup& other);
    AcquisitionSetup& operator=(const AcquisitionSetup& other);
    AcquisitionSetup(AcquisitionSetup&& other) = default;
    AcquisitionSetup& operator=(AcquisitionSetup&& other) = default;

    void addView(View view);
    void applyPreparationProtocol(const AbstractPreparationProtocol& preparation);
    void clearViews(bool keepTimeStamps = false);
    bool isValid() const;
    uint nbViews() const;
    void prepareView(uint viewNb);
    void removeAllPrepareSteps();
    bool resetSystem(const CTsystem& system);
    bool resetSystem(CTsystem&& system);
    void setNbViews(uint nbViews);
    SimpleCTsystem* system();
    const SimpleCTsystem* system() const;
    View& view(uint viewNb);
    const View& view(uint viewNb) const;
    std::vector<View>& views();
    const std::vector<View>& views() const;

    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

private:
    std::unique_ptr<SimpleCTsystem> _system;
    std::vector<View> _views;
};

} // namespace CTL

#endif // ACQUISITIONSETUP_H
