#ifndef CTL_XRAYLASER_H
#define CTL_XRAYLASER_H

#include "abstractsource.h"

namespace CTL {

constexpr double ELEC_VOLT = 1.6021766208e-19; // in Joule (=Ws)

/*!
 * \brief The XrayLaser class represents source components that emit monoenergetic radiation.
 */
class XrayLaser : public AbstractSource
{
    CTL_TYPE_ID(310)

    // implementation of abstract interface
    public: EnergyRange nominalEnergyRange() const override;
    protected: double nominalPhotonFlux() const override;

public:
    XrayLaser(const QString& name);
    XrayLaser(double energy, double output, const QString &name = defaultName());
    XrayLaser(const QSizeF &focalSpotSize = QSizeF(0.0, 0.0),
              const Vector3x1& focalSpotPosition = Vector3x1(0.0),
              double energy = 100.0,
              double output = 1.0,
              const QString &name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    uint spectrumDiscretizationHint() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter
    double photonEnergy() const;
    double radiationOutput() const;

    // setter
    void setPhotonEnergy(double energy);
    void setRadiationOutput(double output);

    // static methods
    static QString defaultName();


protected:
    double _energy; //!< Energy of the emitted photons (in keV).
    double _output; //!< Total emission output (in mWs).

private:
    using AbstractSource::setSpectrumModel;
};

} // namespace CTL

#endif // CTL_XRAYLASER_H
