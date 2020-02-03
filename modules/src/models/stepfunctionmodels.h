#ifndef STEPFUNCTIONMODELS_H
#define STEPFUNCTIONMODELS_H

#include "abstractdatamodel.h"

/*
 * All step functions are implemented with left-closed and right-open intervals,
 * i.e. they are piecewise constant on intervals of the form [a,b).
 * This allows seamless compositions of these models.
 */

namespace CTL {

class StepFunctionModel : public AbstractDataModel
{
    CTL_TYPE_ID(50)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    enum StepDirection{ RightIsZero = 0, Downwards = 0, LeftIsZero = 1, Upwards = 1 };

    StepFunctionModel(float threshold = 0.0f, float amplitude = 1.0f, StepDirection stepDirection = LeftIsZero);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    float _threshold;
    float _amplitude;
    StepDirection _stepDirection;
};

class ConstantModel : public AbstractDataModel
{
    CTL_TYPE_ID(51)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    ConstantModel(float amplitude = 1.0f);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    float _amplitude;
};

class RectFunctionModel : public AbstractDataModel
{
    CTL_TYPE_ID(52)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    RectFunctionModel(float rectBegin = -0.5f, float rectEnd = 0.5f, float amplitude = 1.0f);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    float _rectBegin;
    float _rectEnd;
    float _amplitude;
};

} // namespace CTL

#endif // STEPFUNCTIONMODELS_H
