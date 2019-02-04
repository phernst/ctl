#ifndef DETECTORSATURATIONMODELS_H
#define DETECTORSATURATIONMODELS_H

#include "abstractdatamodel.h"
#include <float.h>

namespace CTL {

/*!
 * \class DetectorSaturationLinearModel
 * \brief The DetectorSaturationLinearModel is a data model to map true (or expected) values to
 * actually measured values with a dependency composed of a linear central segment that connects two
 * constant regimes.
 *
 * The model is defined by two parameters:
 * \li lower saturation level \f$ a \f$
 * \li upper saturation level \f$ b \f$
 *
 * This is illustrated in the following figure.
 * ![Illustration of a DetectorSaturationLinearModel with parameters (2, 8)](linearSaturation.svg)
 */

/*!
 * \class DetectorSaturationSplineModel
 * \brief The DetectorSaturationSplineModel is a data model to map true (or expected) values to
 * actually measured values with a dependency composed of a linear central segment and a fade in/out
 * from/to a constant level by quadratic splines.
 *
 * The model is defined by four parameters:
 * \li lower saturation level \f$ a \f$
 * \li upper saturation level \f$ b \f$
 * \li softness of lower transition \f$ s_a \f$
 * \li softness of upper transition \f$ s_b \f$
 *
 * This is illustrated in the following figure.
 * ![Illustration of a DetectorSaturationSplineModel with parameters (2, 8, 0.5, 2)](splineSaturation.svg)
 */

class DetectorSaturationLinearModel : public AbstractDataModel
{
    ADD_TO_MODEL_ENUM(10)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    DetectorSaturationLinearModel(float lowerCap = 0.0f, float upperCap = FLT_MAX);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    float _a = 0.0f;    //!< lower saturation level
    float _b = FLT_MAX; //!< upper saturation level

    void setParFromList(const QVariantList& list);
    void setParFromMap(const QVariantMap& map);
};

class DetectorSaturationSplineModel : public AbstractDataModel
{
    ADD_TO_MODEL_ENUM(20)

    public: float valueAt(float position) const override;
    public: AbstractDataModel* clone() const override;

public:
    DetectorSaturationSplineModel(float lowerCap = 0.0f, float upperCap = FLT_MAX, float softening = 0.0f);
    DetectorSaturationSplineModel(float lowerCap, float upperCap, float softLower, float softUpper);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

private:
    float _a = 0.0f;     //!< lower saturation level
    float _b = FLT_MAX;  //!< upper saturation level
    float _softA = 0.0f; //!< softening margin of lower saturation
    float _softB = 0.0f; //!< softening margin of upper saturation

    void setParFromList(const QVariantList& list);
    void setParFromMap(const QVariantMap& map);

    float spline1(float x) const;
    float spline2(float x) const;
};



}

#endif // DETECTORSATURATIONMODELS_H
