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

    void setParFromList(const QVariantList& list);
    void setParFromMap(const QVariantMap& map);
};

class DetectorSaturationSplineModel : public AbstractDataModel
{
    public: QVariant toVariant() const override;
    public: void fromVariant(const QVariant &variant) override;
    public: float valueAt(float position) const override;

public:
    DetectorSaturationSplineModel(float lowerCap = 0.0f, float upperCap = FLT_MAX, float softening = 0.1f);
    DetectorSaturationSplineModel(float lowerCap, float upperCap, float softLower, float softUpper);

    QVariant parameter() const override;
    void setParameter(QVariant parameter) override;

private:
    float _a = 0.0f;
    float _b = FLT_MAX;
    float _softA = 0.0f;
    float _softB = 0.0f;

    void setParFromList(const QVariantList& list);
    void setParFromMap(const QVariantMap& map);

    float spline1(float x) const;
    float spline2(float x) const;
};

}

#endif // DETECTORSATURATIONMODELS_H
