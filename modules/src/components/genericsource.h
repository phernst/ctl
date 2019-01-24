#ifndef GENERICSOURCE_H
#define GENERICSOURCE_H

#include "abstractsource.h"
#include <QMap>

#include "models/xrayspectrummodels.h"

namespace CTL {

class GenericSource : public AbstractSource
{
    ADD_TO_COMPONENT_ENUM(301)

    // implementation of abstract interface
    public: IntervalDataSeries spectrum(float from, float to, uint nbSamples) const override;
    protected: double nominalPhotonFlux() const override;

public:
    GenericSource(const QString& name);
    GenericSource(const QSizeF& focalSpotSize = QSizeF(0.0, 0.0),
                  const Vector3x1& focalSpotPosition = Vector3x1(0.0f),
                  const QString& name = defaultName());
    GenericSource(const QJsonObject& json);

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override; // JSON
    void write(QJsonObject& json) const override; // JSON

    // setter methods
    void setSpectrum(const IntervalDataSeries& spectrum, bool updateFlux = false);
    void setPhotonFlux(double flux);

    // static methods
    static QString defaultName();

protected:
    double _totalFlux = 0.0f;
};

inline SystemComponent* GenericSource::clone() const { return new GenericSource(*this); }

inline IntervalDataSeries GenericSource::spectrum(float from, float to, uint nbSamples) const
{
    IntervalDataSeries sampSpec = IntervalDataSeries::sampledFromModel(*_spectrumModel, from, to, nbSamples);
    //sampSpec.normalize();
    return sampSpec;
}

inline double GenericSource::nominalPhotonFlux() const { return _totalFlux; }

inline void GenericSource::setPhotonFlux(double flux) { _totalFlux = flux; }

} // namespace CTL

#endif // GENERICSOURCE_H
