#include "xraytube.h"

namespace CTL {

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

SampledDataSeries XrayTube::spectrum() const
{
    return spectrum(0.0f, _tubeVoltage, DEFAULT_SPECTRUM_RESOLUTION);
}

QString XrayTube::defaultName()
{
    static const QString defName(QStringLiteral("X-ray tube"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

SampledDataSeries XrayTube::spectrum(float from, float to, uint nbSamples) const
{
    if(!_spectrumModel)
        throw std::runtime_error("No spectrum model set.");

    static_cast<AbstractXraySpectrumModel*>(_spectrumModel.get())->setParameter(_tubeVoltage);
    SampledDataSeries spec(from, to, nbSamples, *_spectrumModel);
    spec.normalize();
    return spec;
}

double XrayTube::nominalPhotonFlux() const
{
    return _emissionCurrent * _intensityConstant;
}

} // namespace CTL
