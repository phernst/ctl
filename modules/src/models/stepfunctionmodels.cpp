#include "stepfunctionmodels.h"
#include <QDebug>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(StepFunctionModel)
DECLARE_SERIALIZABLE_TYPE(ConstantModel)
DECLARE_SERIALIZABLE_TYPE(RectFunctionModel)

StepFunctionModel::StepFunctionModel(float threshold, float amplitude, StepDirection stepDirection)
    : _threshold(threshold)
    , _amplitude(amplitude)
    , _stepDirection(stepDirection)
{
}

float StepFunctionModel::valueAt(float position) const
{
    if(position < _threshold)
        return _amplitude * float(_stepDirection == RightIsZero);

    return _amplitude * float(_stepDirection == LeftIsZero);
}

AbstractDataModel* StepFunctionModel::clone() const
{
    return new StepFunctionModel(*this);
}

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with three key-value-pairs, representing threshold, amplitude, and
 * which side of the step is zero.
 */
QVariant StepFunctionModel::parameter() const
{
    QVariantMap ret;
    ret.insert("Threshold", _threshold);
    ret.insert("Amplitude", _amplitude);
    ret.insert("Left is zero", bool(_stepDirection));

    return ret;
}

/*!
 * Sets the parameters of this instance based on the passed QVariant \a parameter.
 *
 * Parameters can be passed by either of the following two options:
 *
 * 1. As a QVariantMap with three key-value-pairs: ("Threshold", \a threshold),
 * ("Amplitude", \a amplitude), and ("Left is zero", \a stepDirection)
 * 2. As a QVariantList: In this case, the list must contain two floating point values and one
 * boolean sorted in the following order: \a threshold, \a amplitude, \a stepDirection.
 */
void StepFunctionModel::setParameter(const QVariant& parameter)
{
    if(parameter.canConvert(QMetaType::QVariantMap))
    {
        auto parMap = parameter.toMap();
        _threshold = parMap.value("Threshold").toFloat();
        _amplitude = parMap.value("Amplitude").toFloat();
        _stepDirection = StepDirection(parMap.value("Left is zero").toBool());
    }
    else if(parameter.canConvert(QMetaType::QVariantList))
    {
        auto parList = parameter.toList();
        if(parList.size() < 3)
        {
            qWarning() << "StepFunctionModel::setParameter: Could not set parameters! "
                          "reason: contained QVariantList has too few entries (required: 2 float, 1 bool)";
            return;
        }
        _threshold = parList.at(0).toFloat();
        _amplitude = parList.at(1).toFloat();
        _stepDirection = StepDirection(parList.at(2).toBool());
    }
    else
        qWarning() << "StepFunctionModel::setParameter: Could not set parameters! "
                      "reason: incompatible variant passed";
}


ConstantModel::ConstantModel(float amplitude)
    : _amplitude(amplitude)
{
}

float ConstantModel::valueAt(float) const { return _amplitude; }

AbstractDataModel* ConstantModel::clone() const { return new ConstantModel(*this); }

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with one key-value-pair, representing amplitude.
 */
QVariant ConstantModel::parameter() const
{
    QVariantMap ret;
    ret.insert("Amplitude", _amplitude);

    return ret;
}

/*!
 * Sets the parameters of this instance based on the passed QVariant \a parameter.
 *
 * Parameters can be passed by either of the following two options:
 *
 * 1. As a QVariantMap with one key-value-pair: ("amplitude", \a amplitude).
 * 2. As a QVariantList: In this case, the list must contain one floating point value, i.e. \a amplitude.
 */
void ConstantModel::setParameter(const QVariant& parameter)
{
    if(parameter.canConvert(QMetaType::QVariantMap))
    {
        auto parMap = parameter.toMap();
        _amplitude = parMap.value("Amplitude").toFloat();
    }
    else if(parameter.canConvert(QMetaType::QVariantList))
    {
        auto parList = parameter.toList();
        if(parList.size() < 1)
        {
            qWarning() << "ConstantModel::setParameter: Could not set parameters! "
                          "reason: contained QVariantList has too few entries (required: 1 float)";
            return;
        }
        _amplitude = parList.at(0).toFloat();
    }
    else
        qWarning() << "ConstantModel::setParameter: Could not set parameters! "
                      "reason: incompatible variant passed";
}

RectFunctionModel::RectFunctionModel(float rectBegin, float rectEnd, float amplitude)
    : _rectBegin(rectBegin)
    , _rectEnd(rectEnd)
    , _amplitude(amplitude)
{
}

float RectFunctionModel::valueAt(float position) const
{
    if(position < _rectBegin)
        return 0.0f;
    if(position < _rectEnd)
        return _amplitude;

    return 0.0f;
}

AbstractDataModel* RectFunctionModel::clone() const
{
    return new RectFunctionModel(*this);
}

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with three key-value-pairs, representing start and end of the rect-interval
 * as well as the amplitude.
 */
QVariant RectFunctionModel::parameter() const
{
    QVariantMap ret;
    ret.insert("Rect begin", _rectBegin);
    ret.insert("Rect end", _rectEnd);
    ret.insert("Amplitude", _amplitude);

    return ret;
}

/*!
 * Sets the parameters of this instance based on the passed QVariant \a parameter.
 *
 * Parameters can be passed by either of the following two options:
 *
 * 1. As a QVariantMap with three key-value-pairs: ("Rect begin", \a rectBegin),
 * ("Rect end", \a rectEnd), and ("Amplitude", \a amplitude)
 * 2. As a QVariantList: In this case, the list must contain three floating point values sorted in
 * the following order: \a rectBegin, \a rectEnd, \a amplitude.
 */
void RectFunctionModel::setParameter(const QVariant& parameter)
{
    if(parameter.canConvert(QMetaType::QVariantMap))
    {
        auto parMap = parameter.toMap();
        _rectBegin = parMap.value("Rect begin").toFloat();
        _rectEnd = parMap.value("Rect end").toFloat();
        _amplitude = parMap.value("Amplitude").toFloat();
    }
    else if(parameter.canConvert(QMetaType::QVariantList))
    {
        auto parList = parameter.toList();
        if(parList.size() < 3)
        {
            qWarning() << "RectFunctionModel::setParameter: Could not set parameters! "
                          "reason: contained QVariantList has too few entries (required: 3 float)";
            return;
        }
        _rectBegin = parList.at(0).toFloat();
        _rectEnd = parList.at(1).toFloat();
        _amplitude = parList.at(2).toFloat();
    }
    else
        qWarning() << "RectFunctionModel::setParameter: Could not set parameters! "
                      "reason: incompatible variant passed";
}

} // namespace CTL
