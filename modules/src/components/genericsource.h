#ifndef GENERICSOURCE_H
#define GENERICSOURCE_H

#include "abstractsource.h"
#include "models/xrayspectrummodels.h"

namespace CTL {

class GenericSource : public AbstractSource
{
    CTL_TYPE_ID(301)

    // implementation of abstract interface
    public:virtual EnergyRange energyRange() const override;
    protected:virtual double nominalPhotonFlux() const override;

public:
    GenericSource(const QString& name);
    GenericSource(const QSizeF& focalSpotSize = QSizeF(0.0, 0.0),
                  const Vector3x1& focalSpotPosition = Vector3x1(0.0f),
                  const QString& name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    uint spectrumDiscretizationHint() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // setter methods
    void setEnergyRange(const EnergyRange& range);
    void setSpectrum(const IntervalDataSeries& spectrum, bool updateFlux = false);
    void setPhotonFlux(double flux);

    // static methods
    static QString defaultName();

protected:
    EnergyRange _energyRange = { 0.0f, 0.0f }; //!< Energy range of the emitted radiation.
    uint _samplingHint = 0; //!< Number of samples from last set spectrum.
    double _totalFlux = 0.0f; //!< Total photon flux (photons/cm² in 1m distance).

};

} // namespace CTL

#endif // GENERICSOURCE_H
