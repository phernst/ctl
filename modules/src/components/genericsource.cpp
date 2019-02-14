#include "genericsource.h"
#include <QDebug>
#include <stdexcept>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(GenericSource)

/*!
 * Constructs a GenericSource with the name \a name.
 *
 * Focal spot size defaults to QSizeF(0.0, 0.0) and the focal spot position to Vector3x1(0.0f).
 */
GenericSource::GenericSource(const QString& name)
    : AbstractSource(name)
{
}

/*!
 * Constructs a GenericSource with a focal spot size of \a focalSpotSize, the focal spot positioned
 * at \a focalSpotPosition and the name \a name.
 */
GenericSource::GenericSource(const QSizeF& focalSpotSize,
                             const Vector3x1& focalSpotPosition,
                             const QString& name)
    : AbstractSource(focalSpotSize, focalSpotPosition, name)
{
}

/*!
 * Returns the default name for the component: "Generic source".
 */
QString GenericSource::defaultName()
{
    static const QString defName(QStringLiteral("Generic source"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class (SystemComponent), the info string contains
 * the following details:
 * \li Total photon flux
 */
QString GenericSource::info() const
{
    QString ret(AbstractSource::info());

    ret += typeInfoString(typeid(this)) +
            "\tTotal photon flux: "   + QString::number(_totalFlux) + "\n";

    ret += (this->type() == GenericSource::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void GenericSource::fromVariant(const QVariant& variant)
{
    AbstractSource::fromVariant(variant);

    _totalFlux = variant.toMap().value("photon flux").toDouble();
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant GenericSource::toVariant() const
{
    QVariantMap ret = AbstractSource::toVariant().toMap();

    ret.insert("photon flux", _totalFlux);

    return ret;
}

/*!
 * Sets the spectrum of this instance to the sampled data provided by \a spectrum.
 *
 * If \a updateFlux = \c true, the total flux will be set to the integral over the samples from
 * \a spectrum. Otherwise, the total flux remains unchanged.
 */
void GenericSource::setSpectrum(const IntervalDataSeries& spectrum, bool updateFlux)
{
    auto specModel = new FixedXraySpectrumModel;

    QMap<float, float> dataMap;
    for(uint smp = 0; smp < spectrum.nbSamples(); ++smp)
        dataMap.insert(spectrum.data().at(smp).x(), spectrum.data().at(smp).y());

    TabulatedDataModel spectrumData;
    spectrumData.setData(std::move(dataMap));

    specModel->setLookupTable(std::move(spectrumData));

    setSpectrumModel(specModel);

    if(updateFlux)
        _totalFlux = spectrum.integral();
}

// use documentation of base class
SystemComponent* GenericSource::clone() const { return new GenericSource(*this); }

/*!
 * Returns the emitted radiation spectrum sampled with \a nbSamples bins covering the energy
 * range of [\a from, \a to] keV. Each energy bin is defined to represent the integral over the
 * contribution of all energies within the bin to the total intensity. The spectrum provides
 * relative intensities, i.e. the sum over all bins equals to one.
 */
IntervalDataSeries GenericSource::spectrum(float from, float to, uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");

    IntervalDataSeries sampSpec = IntervalDataSeries::sampledFromModel(*spectrumModel(), from, to,
                                                                       nbSamples);
    sampSpec.normalizeByIntegral();
    return sampSpec;
}

/*!
 * Returns the nominal photon flux.
 */
double GenericSource::nominalPhotonFlux() const { return _totalFlux; }

/*!
 * Sets total photon flux to \a flux.
 */
void GenericSource::setPhotonFlux(double flux) { _totalFlux = flux; }

} // namespace CTL
