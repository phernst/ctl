#ifndef CTL_ATTENUATIONFILTER_H
#define CTL_ATTENUATIONFILTER_H

#include "abstractbeammodifier.h"
#include "io/ctldatabase.h"

namespace CTL {
/*!
 * \class AttenuationFilter
 *
 * \brief Monomaterial beam filter, based on Lambert-Beer law.
 *
 * This is modifier changes the spectrum according to the absorption properties of a single material
 * of a certain thickness, according to Lambert-Beer's law.
 */

class AttenuationFilter : public AbstractBeamModifier
{
    CTL_TYPE_ID(410)

    // interface
    public:virtual IntervalDataSeries modifiedSpectrum(const IntervalDataSeries& inputSpectrum) override;
    public:virtual double modifiedFlux(double inputFlux,
                                       const IntervalDataSeries& inputSpectrum) override;

public:
    AttenuationFilter(database::Composite material, float mm);
    AttenuationFilter(database::Element material, float mm);
    AttenuationFilter(std::shared_ptr<AbstractIntegrableDataModel> attenuationModel, float mm, float density);

    // SerializationInterface interface
public:
    SystemComponent* clone() const override;
    void fromVariant(const QVariant &variant) override;
    QVariant toVariant() const override;

    // SystemComponent interface
public:
    QString info() const override;

private:
    AttenuationFilter();

    void attenuateSpectrum(IntervalDataSeries& inputSpectrum);

    std::shared_ptr<AbstractIntegrableDataModel> _attenuationModel;
    float _mm;
    float _density;

};

} // namespace CTL

#endif // CTL_ATTENUATIONFILTER_H
