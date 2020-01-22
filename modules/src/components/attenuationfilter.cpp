#include "attenuationfilter.h"
#include <cmath>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(AttenuationFilter)

AttenuationFilter::AttenuationFilter(std::shared_ptr<AbstractIntegrableDataModel> attenuationModel, float mm, float density)
    : _attenuationModel(std::move(attenuationModel))
    , _mm(mm)
    , _density(density)
{
    if(_density < 0.0f)
        qWarning() << "Invalid density value for attenuation filter.";
}

AttenuationFilter::AttenuationFilter(database::Composite material, float mm)
    : AttenuationFilter(database::attenuationModel(material), mm, database::density(material))
{
}

AttenuationFilter::AttenuationFilter(database::Element material, float mm)
    : AttenuationFilter(database::attenuationModel(material), mm, database::density(material))
{
}

AttenuationFilter::AttenuationFilter()
    : AttenuationFilter(nullptr, 0.0f, 0.0f)
{
}

SystemComponent* AttenuationFilter::clone() const
{
    return new AttenuationFilter(*this);
}

void AttenuationFilter::attenuateSpectrum(IntervalDataSeries &inputSpectrum)
{
    const auto binWidth = inputSpectrum.binWidth();

    auto attenuate = [this, binWidth] (QPointF& value)
    {
        const auto E  = value.x();
        const auto i0 = value.y();
        const auto mu = _attenuationModel->meanValue(E, binWidth);

        value.setY(i0 * std::exp(-_mm * mu * 0.1f * _density));
    };

    for(auto& bin : inputSpectrum.data())
        attenuate(bin);
}

IntervalDataSeries AttenuationFilter::modifiedSpectrum(const IntervalDataSeries& inputSpectrum)
{
    IntervalDataSeries modifiedSpectrum(inputSpectrum);

    attenuateSpectrum(modifiedSpectrum);
    modifiedSpectrum.normalizeByIntegral();

    return modifiedSpectrum;
}

double AttenuationFilter::modifiedFlux(double inputFlux, const IntervalDataSeries& inputSpectrum)
{
    IntervalDataSeries modifiedSpectrum(inputSpectrum);

    attenuateSpectrum(modifiedSpectrum);

    auto inputSpectrumIntegral = inputSpectrum.integral();
    if(qFuzzyIsNull(inputSpectrumIntegral))
    {
        qWarning("Input spectrum has integral 0. Still assuming normalized spectrum.");
        inputSpectrumIntegral = 1.0f;
    }
    auto fluxRatio = modifiedSpectrum.integral() / inputSpectrumIntegral;

    return inputFlux * fluxRatio;
}

void AttenuationFilter::fromVariant(const QVariant &variant)
{
    auto varMap = variant.toMap();

    _mm = varMap.value("thickness").toFloat();
    _density = varMap.value("density").toFloat();
    auto model = dynamic_cast<AbstractIntegrableDataModel*>(SerializationHelper::parseDataModel(varMap.value("attenuation model")));
    if(!model)
        qCritical() << "AttenuationFilter could not be deserialized from QVariant."
                       "Contained model is not castable to AbstractIntegrableDataModel.";
    _attenuationModel.reset(std::move(model));

}

QVariant AttenuationFilter::toVariant() const
{
    QVariantMap ret;
    ret.insert("thickness", _mm);
    ret.insert("density", _density);
    ret.insert("attenuation model", _attenuationModel->toVariant());

    return ret;
}

QString AttenuationFilter::info() const
{
    QString ret(AbstractBeamModifier::info());

    ret +=
        typeInfoString(typeid(this)) +
        "\tFilter thickness: " + QString::number(_mm) + " mm\n"
        "\tMaterial density: " + QString::number(_density) + " g/cm^-3\n"
        "\tAttenuation model name: " + _attenuationModel->name() + "\n";

    ret += (this->type() == AttenuationFilter::Type) ? "}\n" : "";

    return ret;
}


} // namespace CTL
