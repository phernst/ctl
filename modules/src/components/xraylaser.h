#ifndef XRAYLASER_H
#define XRAYLASER_H

#include "abstractsource.h"

namespace CTL
{

class XrayLaser : public AbstractSource
{
    ADD_TO_COMPONENT_ENUM(310)

    // implementation of abstract interface
    public: SampledDataSeries spectrum(float from, float to, uint nbSamples) const override;
    protected: double nominalPhotonFlux() const override;

public:
    XrayLaser(const QString& name);
    XrayLaser(double energy, double power, const QString &name = defaultName());
    XrayLaser(const QSizeF &focalSpotSize = QSizeF(0.0, 0.0),
              const Vector3x1& focalSpotPosition = Vector3x1(0.0f),
              double energy = 100.0f,
              double power = 1.0f,
              const QString &name = defaultName());
    XrayLaser(const QJsonObject& json);

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // getter
    double photonEnergy() const;

    // setter
    void setPhotonEnergy(double energy);
    void setPower(double power);

    // static methods
    static QString defaultName();


protected:
    double _energy;
    double _power;

private:
    using AbstractSource::setSpectrumModel;
};

inline SystemComponent* XrayLaser::clone() const { return new XrayLaser(*this); }

inline double XrayLaser::nominalPhotonFlux() const { return _power; }

inline double XrayLaser::photonEnergy() const { return _energy; }

inline void XrayLaser::setPhotonEnergy(double energy) { _energy = energy; }

inline void XrayLaser::setPower(double power) { _power = power; }


}

#endif // XRAYLASER_H
