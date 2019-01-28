#ifndef DETECTORSATURATIONMODELS_H
#define DETECTORSATURATIONMODELS_H

#include "abstractdatamodel.h"
#include <float.h>

namespace CTL {

class DetectorSaturationLinearModel : public AbstractDataModel
{
    public: QVariant toVariant() const override;
    public: void fromVariant(const QVariant &variant) override;
    public: float valueAt(float position) const override;

public:
    DetectorSaturationLinearModel(float lowerCap = 0.0f, float upperCap = FLT_MAX);

    QVariant parameter() const override;
    void setParameter(QVariant parameter) override;

private:
    float _a = 0.0f;
    float _b = FLT_MAX;

    void setParFromList(const QVariantList& map);
    void setParFromMap(const QVariantMap& map);
};

}

#endif // DETECTORSATURATIONMODELS_H
