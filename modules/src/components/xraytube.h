#ifndef XRAYTUBE_H
#define XRAYTUBE_H

#include "abstractsource.h"

namespace CTL {

class XrayTube : public AbstractSource
{   
    CTL_TYPE_ID(320)

    // implementation of abstract interface
    public:virtual EnergyRange energyRange() const override;
    protected:virtual double nominalPhotonFlux() const override;

public:
    XrayTube(const QString &name);
    XrayTube(const QSizeF &focalSpotSize = QSizeF(0.0, 0.0),
             const Vector3x1 &focalSpotPosition = Vector3x1(0.0),
             double tubeVoltage = 100.0,
             double mAs = 1.0,
             const QString &name = defaultName());
    XrayTube(const QSizeF &focalSpotSize, double tubeVoltage, double mAs, const QString &name = defaultName());
    XrayTube(double tubeVoltage, double mAs, const QString &name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization
    //void setSpectrumModel(AbstractXraySpectrumModel* model) override; //deprecated
    uint spectrumDiscretizationHint() const override;

    // getter methods
    double tubeVoltage() const;
    double mAs() const;
    //IntervalDataSeries spectrum() const; // convenience alternative

    // setter methods
    void setTubeVoltage(double voltage);
    void setMilliampereSeconds(double mAs);
    // void setIntensityConstant(double value); //deprecated



    // static methods
    static QString defaultName();

protected:
    double _tubeVoltage       = 100.0f;
    double _mAs               = 10.0f;
    double _intensityConstant = 3.2e8; // Default intensity constant [PHOTONS / (mAs * cm^2)]

private:
    void updateIntensityConstant();

    void setSpectrumModel(AbstractXraySpectrumModel* model) override;
};

} // namespace CTL

#endif // XRAYTUBE_H
