#ifndef GENERICSOURCE_H
#define GENERICSOURCE_H

#include "abstractsource.h"
#include "models/xrayspectrummodels.h"

namespace CTL {

class GenericSource : public AbstractSource
{
    CTL_TYPE_ID(301)

    // implementation of abstract interface
    public: IntervalDataSeries spectrum(float from, float to, uint nbSamples) const override;
    protected: double nominalPhotonFlux() const override;

public:
    GenericSource(const QString& name);
    GenericSource(const QSizeF& focalSpotSize = QSizeF(0.0, 0.0),
                  const Vector3x1& focalSpotPosition = Vector3x1(0.0f),
                  const QString& name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // setter methods
    void setSpectrum(const IntervalDataSeries& spectrum, bool updateFlux = false);
    void setPhotonFlux(double flux);

    // static methods
    static QString defaultName();

protected:
    double _totalFlux = 0.0f;
};

} // namespace CTL

#endif // GENERICSOURCE_H
