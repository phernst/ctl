#ifndef XRAYTUBE_H
#define XRAYTUBE_H

#include "models/xrayspectrummodels.h"
#include "abstractsource.h"

namespace CTL
{

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

inline SystemComponent* XrayTube::clone() const { return new XrayTube(*this); }

inline double XrayTube::tubeVoltage() const { return _tubeVoltage; }

inline double XrayTube::emissionCurrent() const { return _emissionCurrent; }

inline void XrayTube::setTubeVoltage(double voltage) { _tubeVoltage = voltage; }

inline void XrayTube::setEmissionCurrent(double current) { _emissionCurrent = current; }

inline void XrayTube::setIntensityConstant(double value) { _intensityConstant = value; }

inline void XrayTube::setSpectrumModel(AbstractXraySpectrumModel *model)
{
    if(!dynamic_cast<AbstractXraySpectrumModel*>(model))
        throw std::runtime_error("Spectral model could not be set: not of type AbstractXraySpectrumModel");

    _spectrumModel.reset(model);
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void XrayTube::read(const QJsonObject &json)
{
    AbstractSource::read(json);

    _tubeVoltage = json.value("tube voltage").toDouble();
    _emissionCurrent = json.value("emission current").toDouble();
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
inline void XrayTube::write(QJsonObject &json) const
{
    AbstractSource::write(json);

    json.insert("tube voltage", _tubeVoltage);
    json.insert("emission current", _emissionCurrent);
}


}

#endif // XRAYTUBE_H
