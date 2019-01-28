#include "detectorsaturationmodels.h"
#include <QDebug>

namespace CTL {

DetectorSaturationLinearModel::DetectorSaturationLinearModel(float lowerCap,
                                                             float upperCap)
    : _a(lowerCap)
    , _b(upperCap)
{
}

QVariant DetectorSaturationLinearModel::toVariant() const
{
    QVariantMap ret;

    QVariantMap pars;
    pars.insert("a", _a);
    pars.insert("b", _b);

    ret.insert("name", "DetectorSaturationLinearModel");
    ret.insert("parameters", pars);

    return ret;
}

void DetectorSaturationLinearModel::fromVariant(const QVariant &variant)
{
    auto map = variant.toMap();
    if(!map.value("name").toString().compare("DetectorSaturationLinearModel"))
    {
        qWarning() << "DetectorSaturationLinearModel::fromVariant: Could not construct instance! "
                      "reason: incompatible variant passed";
        return;
    }

    setParameter(variant);
}

float DetectorSaturationLinearModel::valueAt(float position) const
{
    if(position < _a)
        return _a;
    else if(position > _b)
        return _b;
    else
        return position;
}

QVariant DetectorSaturationLinearModel::parameter() const
{
    QVariantMap ret;
    ret.insert("a", _a);
    ret.insert("b", _b);

    return ret;
}

void DetectorSaturationLinearModel::setParameter(QVariant parameter)
{
    if(parameter.canConvert(QMetaType::QVariantMap))
        setParFromMap(parameter.toMap());
    else if(parameter.canConvert(QMetaType::QVariantList))
        setParFromList(parameter.toList());
    else
        qWarning() << "DetectorSaturationLinearModel::setParameter: Could not set parameters! "
                      "reason: incompatible variant passed";
}

void DetectorSaturationLinearModel::setParFromList(const QVariantList& list)
{
    if(list.size()<2)
    {
        qWarning() << "DetectorSaturationLinearModel::setParameter: Could not set parameters! "
                      "reason: contained QVariantList has less than 2 entries";
        return;
    }
    _a = list.at(0).toFloat();
    _b = list.at(1).toFloat();
}

void DetectorSaturationLinearModel::setParFromMap(const QVariantMap &map)
{
    _a = map.value("a", 0.0f).toFloat();
    _b = map.value("b", FLT_MAX).toFloat();
}

}
