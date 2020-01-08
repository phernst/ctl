#include "detectorsaturationmodels.h"
#include <QDebug>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(DetectorSaturationLinearModel);
DECLARE_SERIALIZABLE_TYPE(DetectorSaturationSplineModel);

// ### DetectorSaturationLinearModel ###

/*!
 * Constructs a DetectorSaturationLinearModel with (lower/upper) saturation levels at \a lowerCap
 * and \a upperCap.
 */
DetectorSaturationLinearModel::DetectorSaturationLinearModel(float lowerCap,
                                                             float upperCap)
    : _a(lowerCap)
    , _b(upperCap)
{
}

/*!
 * Returns the value from the model at \a position.
 *
 * \f$
 *  f(x)=\begin{cases}
 *  a & x<a\\
 *  x & a\leq x\leq b\\
 *  b & x>b
 *  \end{cases}
 *  \f$
 */
float DetectorSaturationLinearModel::valueAt(float position) const
{
    if(position < _a)
        return _a;
    else if(position > _b)
        return _b;
    else
        return position;
}

// Re-use documentation of base class
AbstractDataModel* DetectorSaturationLinearModel::clone() const
{
    return new DetectorSaturationLinearModel(*this);
}

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with two key-value-pairs: ("a",\a a), ("b",\a b), where \a a and \a b
 * denote the lower and upper saturation level, respectively.
 */
QVariant DetectorSaturationLinearModel::parameter() const
{
    QVariantMap ret;
    ret.insert("a", _a);
    ret.insert("b", _b);

    return ret;
}

/*!
 * Sets the parameters of this instance based on the passed QVariant \a parameter.
 *
 * Parameters can be passed by either of the following two options:
 *
 * 1. As a QVariantMap with two key-value-pairs: ("a", \a a), ("b", \a b). In this case, \a a and
 * \a b denote the lower and upper saturation level.
 * 2. As a QVariantList: In this case, the list must contain two floating point values sorted in
 * the following order: \a a, \a b.
 *
 * Example:
 * \code
 * DetectorSaturationLinearModel linearModel;
 * // option 1
 * QVariantMap parameterMap;
 * parameterMap.insert("a", 1.0f);
 * parameterMap.insert("b", 5.0f);
 * linearModel.setParameter(parameterMap);
 *
 * // option 1 - alternative way
 * linearModel.setParameter(QVariantMap{ {"a", 1.0f},
 *                                       {"b", 5.0f} });
 *
 * // option 2
 * linearModel.setParameter(QVariantList{1.0f, 5.0f});
 * \endcode
 */
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

/*!
 * Helper method for setParameter().
 */
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

/*!
 * Helper method for setParameter().
 */
void DetectorSaturationLinearModel::setParFromMap(const QVariantMap &map)
{
    _a = map.value("a", 0.0f).toFloat();
    _b = map.value("b", FLT_MAX).toFloat();
}

// ### DetectorSaturationSplineModel ###

/*!
 * Constructs a DetectorSaturationSplineModel with (lower/upper) saturation levels at \a lowerCap
 * and \a upperCap and equally sized softening margins \f$ s_{a}=s_{b}=softening \f$.
 */
DetectorSaturationSplineModel::DetectorSaturationSplineModel(float lowerCap, float upperCap, float softening)
    : _a(lowerCap)
    , _b(upperCap)
    , _softA(softening)
    , _softB(softening)
{
}

/*!
 * Constructs a DetectorSaturationSplineModel with (lower/upper) saturation levels at \a lowerCap
 * and \a upperCap. Softening margins around the saturation levels are set to \a softLower and
 * \a softUpper, respectively.
 */
DetectorSaturationSplineModel::DetectorSaturationSplineModel(float lowerCap, float upperCap,
                                                             float softLower, float softUpper)
    : _a(lowerCap)
    , _b(upperCap)
    , _softA(softLower)
    , _softB(softUpper)
{
}

// Re-use documentation of base class
AbstractDataModel* DetectorSaturationSplineModel::clone() const
{
    return new DetectorSaturationSplineModel(*this);
}

/*!
 * Returns the value from the model at \a position.
 *
 * \f$
 *  f(x)=\begin{cases}
 *  a & x<a-s_{a}\\
 *  S_{1}(x) & a-s_{a}\leq x<a+s_{a}\\
 *  x & a+s_{a}\leq x<b-s_{b}\\
 *  S_{2}(x) & b-s_{b}\leq x<b+s_{b}\\
 *  b & x\geq b
 *  \end{cases}
 * \f$
 *
 * With:
 *  \f$
 *  \begin{align*}
 *  S_{1}(x) & =\frac{1}{4s_{a}}x^{2}-\frac{a-s_{a}}{2s_{a}}x+\frac{(a+s_{a})^{2}}{4s_{a}}\\
 *  S_{2}(x) & =-\frac{1}{4s_{b}}x^{2}+\frac{b+s_{b}}{2s_{b}}x+\frac{(b-s_{b})^{2}}{4s_{b}}
 *  \end{align*}
 * \f$
 */
float DetectorSaturationSplineModel::valueAt(float position) const
{
    float spl1Start = _a - _softA;
    float spl1End   = _a + _softA;
    float spl2Start = _b - _softB;
    float spl2End   = _b + _softB;

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

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with four key-value-pairs: ("a", \a a), ("b", \a b),
 * ("softA", \f$ s_a \f$), ("softB", \f$ s_b \f$). Here, \a a and \a b denote the lower and upper
 * saturation level, respectively, and \f$ s_a \f$ and \f$ s_b \f$ are the softening margins of
 * the transitions to/from the linear regime.
 */
QVariant DetectorSaturationSplineModel::parameter() const
{
    QVariantMap map;
    map.insert("a", _a);
    map.insert("b", _b);
    map.insert("softA", _softA);
    map.insert("softB", _softB);

    return map;
}

/*!
 * Sets the parameters of this instance based on the passed QVariant \a parameter.
 *
 * Parameters can be passed by either of the following two options:
 *
 * 1. As a QVariantMap with four key-value-pairs: ("a", \a a), ("b", \a b), ("softA", \f$ s_a \f$),
 * ("softB", \f$ s_b \f$). In this case, \a a and \a b denote the lower and upper saturation level
 * and \f$ s_a \f$ and \f$ s_b \f$ specify the softening margins of the transitions to/from the
 * linear regime.
 * 2. As a QVariantList: In this case, the list must contain four floating point values sorted in
 * the following order: \a a, \a b, \f$ s_a \f$, \f$ s_b \f$.
 *
 * Example:
 * \code
 * DetectorSaturationSplineModel splineModel;
 * // option 1
 * QVariantMap parameterMap;
 * parameterMap.insert("a", 1.0f);
 * parameterMap.insert("b", 5.0f);
 * parameterMap.insert("softA", 0.2f);
 * parameterMap.insert("softB", 0.5f);
 * splineModel.setParameter(parameterMap);
 *
 * // option 1 - alternative way
 * splineModel.setParameter(QVariantMap{ {"a", 1.0f},
 *                                       {"b", 5.0f},
 *                                       {"softA", 0.2f},
 *                                       {"softB", 0.5f} });
 *
 * // option 2
 * splineModel.setParameter(QVariantList{1.0f, 5.0f, 0.2f, 0.5f});
 * \endcode
 */
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

/*!
 * Helper method for setParameter().
 */
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

/*!
 * Helper method for setParameter().
 */
void DetectorSaturationSplineModel::setParFromMap(const QVariantMap &map)
{
    _a = map.value("a", 0.0f).toFloat();
    _b = map.value("b", FLT_MAX).toFloat();
    _softA = map.value("softA", 0.0f).toFloat();
    _softB = map.value("softB", 0.0f).toFloat();
}

/*!
 * Helper method for valueAt().
 */
float DetectorSaturationSplineModel::spline1(float x) const
{
    return 1.0f/(4.0f*_softA)*(x*x) - (_a-_softA)/(2.0f*_softA)*x +
            (_a+_softA)*(_a+_softA)/(4.0f*_softA);
}

/*!
 * Helper method for valueAt().
 */
float DetectorSaturationSplineModel::spline2(float x) const
{
    return -1.0f/(4.0f*_softB)*(x*x) + (_b+_softB)/(2.0f*_softB)*(x) -
            (_b-_softB)*(_b-_softB)/(4.0f*_softB);
}

} // namespace CTL
