#include "xraytube.h"

namespace CTL {

DECLARE_JSON_COMPATIBLE_TYPE(XrayTube)

XrayTube::XrayTube(const QSizeF &focalSpotSize,
                   const Vector3x1 &focalSpotPosition,
                   double tubeVoltage,
                   double emissionCurrent,
                   const QString &name)
    : AbstractSource(focalSpotSize, focalSpotPosition, new KramersLawSpectrumModel, name),
      _tubeVoltage(tubeVoltage),
      _emissionCurrent(emissionCurrent)
{
}

XrayTube::XrayTube(const QSizeF &focalSpotSize, double tubeVoltage, double emissionCurrent, const QString &name)
    : XrayTube(focalSpotSize, Vector3x1(0.0), tubeVoltage, emissionCurrent, name)
{
}

XrayTube::XrayTube(double tubeVoltage, double emissionCurrent, const QString &name)
    : XrayTube(QSizeF(0.0,0.0), Vector3x1(0.0), tubeVoltage, emissionCurrent, name)
{
}

XrayTube::XrayTube(const QString &name)
    : XrayTube(QSizeF(0.0,0.0), Vector3x1(0.0), 100.0f, 1.0f, name)
{
}

XrayTube::XrayTube(const QJsonObject& json)
    : AbstractSource(defaultName())
{
    XrayTube::read(json);
}


QString XrayTube::info() const
{
    QString ret(AbstractSource::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tTube voltage: " + QString::number(_tubeVoltage) + " keV\n"
        "\tEmission current: " + QString::number(_emissionCurrent) + " mA\n";
    ret += (this->type() == XrayTube::Type) ? "}\n" : "";

    return ret;
}

IntervalDataSeries XrayTube::spectrum() const
{
    return spectrum(0.0f, _tubeVoltage, DEFAULT_SPECTRUM_RESOLUTION);
}

QString XrayTube::defaultName()
{
    static const QString defName(QStringLiteral("X-ray tube"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

IntervalDataSeries XrayTube::spectrum(float from, float to, uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");
    static_cast<AbstractXraySpectrumModel*>(_spectrumModel.get())->setParameter(_tubeVoltage);
    IntervalDataSeries spec = IntervalDataSeries::sampledFromModel(*spectrumModel(), from, to, nbSamples);
    spec.normalizeByIntegral();
    return spec;
}

double XrayTube::nominalPhotonFlux() const
{
    return _emissionCurrent * _intensityConstant;
}

SystemComponent* XrayTube::clone() const { return new XrayTube(*this); }

double XrayTube::tubeVoltage() const { return _tubeVoltage; }

double XrayTube::emissionCurrent() const { return _emissionCurrent; }

void XrayTube::setTubeVoltage(double voltage) { _tubeVoltage = voltage; }

void XrayTube::setEmissionCurrent(double current) { _emissionCurrent = current; }

void XrayTube::setIntensityConstant(double value) { _intensityConstant = value; }

void XrayTube::setSpectrumModel(AbstractXraySpectrumModel *model)
{
    if(!dynamic_cast<AbstractXraySpectrumModel*>(model))
        throw std::runtime_error("Spectral model could not be set: not of type AbstractXraySpectrumModel");

    _spectrumModel.reset(model);
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void XrayTube::read(const QJsonObject &json)
{
    AbstractSource::read(json);

    _tubeVoltage = json.value("tube voltage").toDouble();
    _emissionCurrent = json.value("emission current").toDouble();
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
void XrayTube::write(QJsonObject &json) const
{
    AbstractSource::write(json);

    json.insert("tube voltage", _tubeVoltage);
    json.insert("emission current", _emissionCurrent);
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void XrayTube::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _tubeVoltage = varMap.value("tube voltage").toDouble();
    _emissionCurrent = varMap.value("emission current").toDouble();
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant XrayTube::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("tube voltage", _tubeVoltage);
    ret.insert("emission current", _emissionCurrent);

    return ret;
}

} // namespace CTL
