#ifndef XRAYTUBE_H
#define XRAYTUBE_H

#include "models/xrayspectrummodels.h"
#include "abstractsource.h"

namespace CTL {

const double DEFAULT_I_CONST = 1.0; // [PHOTONS / (mAs * cm^2)]
const uint   DEFAULT_SPECTRUM_RESOLUTION = 40;

class XrayTube : public AbstractSource
{
    ADD_TO_COMPONENT_ENUM(320)

    // implementation of abstract interface
    public: IntervalDataSeries spectrum(float from, float to, uint nbSamples) const override;
    protected: double nominalPhotonFlux() const override;

public:
    XrayTube(const QString &name);
    XrayTube(const QSizeF &focalSpotSize = QSizeF(0.0, 0.0),
             const Vector3x1 &focalSpotPosition = Vector3x1(0.0f),
             double tubeVoltage = 100.0,
             double emissionCurrent = 1.0,
             const QString &name = defaultName());
    XrayTube(const QSizeF &focalSpotSize, double tubeVoltage, double emissionCurrent, const QString &name = defaultName());
    XrayTube(double tubeVoltage, double emissionCurrent, const QString &name = defaultName());
    XrayTube(const QJsonObject& json);

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void setSpectrumModel(AbstractXraySpectrumModel *model) override;
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // getter methods
    double tubeVoltage() const;
    double emissionCurrent() const;
    IntervalDataSeries spectrum() const; // convenience alternative

    // setter methods
    void setTubeVoltage(double voltage);
    void setEmissionCurrent(double current);
    void setIntensityConstant(double value);

    // static methods
    static QString defaultName();

protected:
    double _tubeVoltage       = 100.0f;
    double _emissionCurrent   = 10.0f;
    double _intensityConstant = DEFAULT_I_CONST;
};

} // namespace CTL

#endif // XRAYTUBE_H
