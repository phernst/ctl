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

    ret.insert("name", "DetectorSaturationLinearModel");
    ret.insert("parameters", parameter().toMap());

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

void DetectorSaturationLinearModel::setParameter(const QVariant &parameter)
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
                      "reason: contained QVariantList has too few entries (required: 2 float)";
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
    QVariantMap ret;

    ret.insert("name", "DetectorSaturationSplineModel");
    ret.insert("parameters", parameter().toMap());

    return ret;
}

void DetectorSaturationSplineModel::fromVariant(const QVariant &variant)
{
    auto map = variant.toMap();
    if(!map.value("name").toString().compare("DetectorSaturationSplineModel"))
    {
        qWarning() << "DetectorSaturationSplineModel::fromVariant: Could not construct instance! "
                      "reason: incompatible variant passed";
        return;
    }

    setParameter(variant);
}

float DetectorSaturationSplineModel::valueAt(float position) const
{
    float spl1Start = _a * (1.0 - _softA);
    float spl1End   = _a * (1.0 + _softA);
    float spl2Start = _b * (1.0 - _softB);
    float spl2End   = _b * (1.0 + _softB);

    if(position < spl1Start)        // lower saturation
        return _a;
    else if(position < spl1End)     // lower spline regime
        return spline1(position);
    else if(position < spl2Start)   // linear regime
        return position;
    else if(position < spl2End)     // upper spline regime
        return spline2(position);
    else                            // upper saturation
        return _b;
}

DetectorSaturationSplineModel::DetectorSaturationSplineModel(float lowerCap, float upperCap, float softening)
    : _a(lowerCap)
    , _b(upperCap)
    , _softA(softening)
    , _softB(softening)
{
}

DetectorSaturationSplineModel::DetectorSaturationSplineModel(float lowerCap, float upperCap,
                                                             float softLower, float softUpper)
    : _a(lowerCap)
    , _b(upperCap)
    , _softA(softLower)
    , _softB(softUpper)
{
}

QVariant DetectorSaturationSplineModel::parameter() const
{
    QVariantMap map;
    map.insert("a", _a);
    map.insert("b", _b);
    map.insert("softA", _softA);
    map.insert("softB", _softB);

    return map;
}

void DetectorSaturationSplineModel::setParameter(const QVariant &parameter)
{
    if(parameter.canConvert(QMetaType::QVariantMap))
        setParFromMap(parameter.toMap());
    else if(parameter.canConvert(QMetaType::QVariantList))
        setParFromList(parameter.toList());
    else
        qWarning() << "DetectorSaturationSplineModel::setParameter: Could not set parameters! "
                      "reason: incompatible variant passed";
}

void DetectorSaturationSplineModel::setParFromList(const QVariantList &list)
{
    if(list.size()<4)
    {
        qWarning() << "DetectorSaturationSplineModel::setParameter: Could not set parameters! "
                      "reason: contained QVariantList has too few entries (required: 4 float)";
        return;
    }
    _a = list.at(0).toFloat();
    _b = list.at(1).toFloat();
    _softA = list.at(2).toFloat();
    _softB = list.at(3).toFloat();
}

void DetectorSaturationSplineModel::setParFromMap(const QVariantMap &map)
{
    _a = map.value("a", 0.0f).toFloat();
    _b = map.value("b", FLT_MAX).toFloat();
    _softA = map.value("softA", 0.0f).toFloat();
    _softB = map.value("softB", 0.0f).toFloat();
}

float DetectorSaturationSplineModel::spline1(float x) const
{
    const float s = _a*_softA;
    return 1.0f/(4.0f*s)*(x*x) - (_a-s)/(2.0f*s)*x + (_a+s)*(_a+s)/(4.0f*s);
}

float DetectorSaturationSplineModel::spline2(float x) const
{
    const float s = _b*_softB;
    return -1.0f/(4.0f*s)*(x*x) + (_b+s)/(2.0f*s)*(x) - (_b-s)*(_b-s)/(4.0f*s);
}

}
