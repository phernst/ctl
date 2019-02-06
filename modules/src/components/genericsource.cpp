#include "genericsource.h"
#include <QDebug>
#include <stdexcept>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(GenericSource)

GenericSource::GenericSource(const QJsonObject& json)
    : AbstractSource(defaultName())
{
    GenericSource::read(json);
}

GenericSource::GenericSource(const QString& name)
    : AbstractSource(name)
{
}

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

QString GenericSource::info() const
{
    QString ret(AbstractSource::info());

    ret += typeInfoString(typeid(this));

    ret += (this->type() == GenericSource::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void GenericSource::read(const QJsonObject& json) { AbstractSource::read(json); }

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
void GenericSource::write(QJsonObject& json) const { AbstractSource::write(json); }

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

SystemComponent* GenericSource::clone() const { return new GenericSource(*this); }

IntervalDataSeries GenericSource::spectrum(float from, float to, uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");

    IntervalDataSeries sampSpec = IntervalDataSeries::sampledFromModel(*spectrumModel(), from, to,
                                                                       nbSamples);
    sampSpec.normalizeByIntegral();
    return sampSpec;
}

double GenericSource::nominalPhotonFlux() const { return _totalFlux; }

void GenericSource::setPhotonFlux(double flux) { _totalFlux = flux; }

} // namespace CTL
