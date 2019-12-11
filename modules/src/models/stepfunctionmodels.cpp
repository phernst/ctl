#include "stepfunctionmodels.h"
#include <QDebug>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(StepFunctionModel)

StepFunctionModel::StepFunctionModel(float threshold, float amplitude, StepDirection stepDirection)
    : _threshold(threshold)
    , _amplitude(amplitude)
    , _stepDirection(stepDirection)
{
}

float StepFunctionModel::valueAt(float position) const
{
    if(position > _threshold)
        return _amplitude * float(_stepDirection == LeftIsZero);
    else
        return _amplitude * float(_stepDirection == RightIsZero);
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
 * 1. As a QVariantMap with three key-value-pairs: ("threshold", \a threshold),
 * ("amplitude", \a amplitude), and ("leftSideZero", \a leftIsZero)
 * 2. As a QVariantList: In this case, the list must contain two floating point values and one
 * boolean sorted in the following order: \a threshold, \a amplitude, \a leftIsZero.
 */
void StepFunctionModel::setParameter(const QVariant &parameter)
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
        if(parList.size()<3)
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

} // namespace CTL
