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
    _spectrumModel.ptr.reset((new XrayLaserSpectrumModel));
    XrayLaser::read(json);
}

IntervalDataSeries XrayLaser::spectrum(float from, float to, uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");

    static_cast<XrayLaserSpectrumModel*>(_spectrumModel.ptr.get())->setParameter(_energy);
    IntervalDataSeries spec = IntervalDataSeries::sampledFromModel(*spectrumModel(), from, to, nbSamples);
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

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void XrayLaser::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _energy = varMap.value("energy").toDouble();
    _power  = varMap.value("power").toDouble();
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant XrayLaser::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("energy", _energy);
    ret.insert("power", _power);

    return ret;
}

QString XrayLaser::defaultName()
{
    static const QString defName(QStringLiteral("X-ray laser"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

SystemComponent* XrayLaser::clone() const { return new XrayLaser(*this); }

double XrayLaser::nominalPhotonFlux() const { return _power; }

double XrayLaser::photonEnergy() const { return _energy; }

void XrayLaser::setPhotonEnergy(double energy) { _energy = energy; }

void XrayLaser::setPower(double power) { _power = power; }

} // namespace CTL
