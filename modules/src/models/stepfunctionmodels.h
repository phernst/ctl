#ifndef STEPFUNCTIONMODELS_H
#define STEPFUNCTIONMODELS_H

#include "abstractdatamodel.h"

namespace CTL {

class StepFunctionModel : public AbstractDataModel
{
    CTL_TYPE_ID(50)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    StepFunctionModel(float threshold = 0.0f, float amplitude = 1.0f, bool leftIsZero = true);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    float _threshold;
    float _amplitude;
    bool _leftIsZero;
};

} // namespace CTL
#endif // STEPFUNCTIONMODELS_H