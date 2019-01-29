#ifndef DETECTORSATURATIONMODELS_H
#define DETECTORSATURATIONMODELS_H

#include "abstractdatamodel.h"
#include <float.h>

namespace CTL {

class DetectorSaturationLinearModel : public AbstractDataModel
{
    ADD_TO_MODEL_ENUM(10)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    DetectorSaturationLinearModel(const QJsonObject& obj);
    DetectorSaturationLinearModel(float lowerCap = 0.0f, float upperCap = FLT_MAX);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    float _a = 0.0f;
    float _b = FLT_MAX;

    void setParFromList(const QVariantList& list);
    void setParFromMap(const QVariantMap& map);
};

class DetectorSaturationSplineModel : public AbstractDataModel
{
    ADD_TO_MODEL_ENUM(20)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    DetectorSaturationSplineModel(const QJsonObject& obj);
    DetectorSaturationSplineModel(float lowerCap = 0.0f, float upperCap = FLT_MAX, float softening = 0.1f);
    DetectorSaturationSplineModel(float lowerCap, float upperCap, float softLower, float softUpper);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

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
