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

QVariant DetectorSaturationSplineModel::toVariant() const
{
    return QVariant();
}

void DetectorSaturationSplineModel::fromVariant(const QVariant &variant)
{

}

float DetectorSaturationSplineModel::valueAt(float position) const
{
    float low  = _a * (1.0 - _soft);
    float high = _b * (1.0 + _soft);

    if(position < low)
        return _a;
    else if(position < _a * (1.0 + _soft))
        return spline1(position);
    else if(position < _b * (1.0 - _soft))
        return position;
    else if(position < high)
        return spline2(position);
    else
        return _b;
}

DetectorSaturationSplineModel::DetectorSaturationSplineModel(float lowerCap, float upperCap, float softening)
    : _a(lowerCap)
    , _b(upperCap)
    , _soft(softening)
{
}

float DetectorSaturationSplineModel::spline1(float x) const
{
    const float s = _a*_soft;
    return 1.0f/(4.0f*s)*(x*x) - (_a-s)/(2.0f*s)*x + (_a+s)*(_a+s)/(4.0f*s);
}

float DetectorSaturationSplineModel::spline2(float x) const
{
    const float s = _b*_soft;
    return -1.0f/(4.0f*s)*(x*x) + (_b+s)/(2.0f*s)*(x) - (_b-s)*(_b-s)/(4.0f*s);
}

}
