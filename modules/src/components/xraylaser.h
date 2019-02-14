#ifndef XRAYLASER_H
#define XRAYLASER_H

#include "abstractsource.h"

namespace CTL {

/*!
 * \brief The XrayLaser class represents source components that emit monoenergetic radiation.
 */
class XrayLaser : public AbstractSource
{
    CTL_TYPE_ID(310)

    // implementation of abstract interface
    public: IntervalDataSeries spectrum(float from, float to, uint nbSamples) const override;
    protected: double nominalPhotonFlux() const override;

public:
    XrayLaser(const QString& name);
    XrayLaser(double energy, double power, const QString &name = defaultName());
    XrayLaser(const QSizeF &focalSpotSize = QSizeF(0.0, 0.0),
              const Vector3x1& focalSpotPosition = Vector3x1(0.0f),
              double energy = 100.0f,
              double power = 1.0f,
              const QString &name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter
    double photonEnergy() const;

    // setter
    void setPhotonEnergy(double energy);
    void setPower(double power);

    // static methods
    static QString defaultName();


protected:
    double _energy; //!< Energy of the emitted photons (in keV).
    double _power;  //!< Total emission power.

private:
    using AbstractSource::setSpectrumModel;
};

} // namespace CTL

#endif // XRAYLASER_H
