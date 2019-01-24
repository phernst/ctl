#include "xraylaser.h"
#include "models/xrayspectrummodels.h"

namespace CTL
{

XrayLaser::XrayLaser(const QSizeF &focalSpotSize,
                     const Vector3x1 &focalSpotPosition,
                     double energy,
                     double power,
                     const QString &name)
    : AbstractSource(focalSpotSize, focalSpotPosition, new XrayLaserSpectrumModel, name)
    , _energy(energy)
    , _power(power)
{
}

XrayLaser::XrayLaser(double energy, double power, const QString &name)
    : XrayLaser(QSizeF(0.0,0.0), Vector3x1(0.0f), energy, power, name)
{
}

XrayLaser::XrayLaser(const QString &name)
    : XrayLaser(QSizeF(0.0,0.0), Vector3x1(0.0f), 100.0f, 1.0f, name)
{
}

XrayLaser::XrayLaser(const QJsonObject& json)
    : AbstractSource(defaultName())

{
    _spectrumModel.reset(new XrayLaserSpectrumModel);
    XrayLaser::read(json);
}

IntervalDataSeries XrayLaser::spectrum(float from, float to, uint nbSamples) const
{
    static_cast<XrayLaserSpectrumModel*>(_spectrumModel.get())->setParameter(_energy);
    IntervalDataSeries spec = IntervalDataSeries::sampledFromModel(*_spectrumModel, from, to, nbSamples);
    spec.normalizeByIntegral();
    return spec;
}

QString XrayLaser::info() const
{
    QString ret(AbstractSource::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tEnergy: " + QString::number(_energy) + " keV\n"
        "\tPower: " +  QString::number(_power) + "\n";
    ret += (this->type() == XrayLaser::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void XrayLaser::read(const QJsonObject &json)
{
    AbstractSource::read(json);

    _energy = json.value("energy").toDouble();
    _power  = json.value("power").toDouble();
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
void XrayLaser::write(QJsonObject &json) const
{
    AbstractSource::write(json);

    json.insert("energy", _energy);
    json.insert("power", _power);
}

QString XrayLaser::defaultName()
{
    static const QString defName(QStringLiteral("X-ray laser"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

}
