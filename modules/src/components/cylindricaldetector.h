#ifndef CYLINDRICALDETECTOR_H
#define CYLINDRICALDETECTOR_H

#include "abstractdetector.h"

namespace CTL {
/*!
 * \class CylindricalDetector
 *
 * \brief Specialized sub-class of AbstractDetector for detector systems with cylindrical
 * arrangement of modules.
 *
 * This class implements a structure to describe cylindrical detector systems. The detector system
 * consists of multiple flat panel modules that are organized in a linear fashion, i.e. the system
 * can be described as one row of modules. In this geometry, all center points of the modules lay on
 * a cylindrical surface. The geometry is specified by the angulation between adjacent modules and
 * the (optional) spacing in between them.
 */

class CylindricalDetector : public AbstractDetector
{
    ADD_TO_COMPONENT_ENUM(110)

    // implementation of abstract interface
    public: QVector<ModuleLocation> moduleLocations() const override;

public:
    CylindricalDetector(const QSize& nbPixelPerModule,
                        const QSizeF& pixelDimensions,
                        uint nbDetectorModules,
                        double angulationPerModule,
                        double moduleSpacing,
                        const QString& name = defaultName());
    CylindricalDetector(const QJsonObject& json);

    static CylindricalDetector fromAngulationAndSpacing(const QSize& nbPixelPerModule,
                                                        const QSizeF& pixelDimensions,
                                                        uint nbDetectorModules,
                                                        double angulationPerModule,
                                                        double moduleSpacing,
                                                        const QString& name = defaultName());
    static CylindricalDetector fromRadiusAndFanAngle(const QSize& nbPixelPerModule,
                                                     const QSizeF& pixelDimensions,
                                                     uint nbDetectorModules,
                                                     double radius,
                                                     double fanAngle,
                                                     const QString& name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // getter methods
    double angulationOfModule(uint module) const;

    // other methods
    double coneAngle() const;
    double curvatureRadius() const;
    double fanAngle() const;
    double rowCoverage() const;

    // static methods
    static QString defaultName();

protected:
    uint _nbModules;
    double _angulationPerModule;
    double _moduleSpacing;

private:
    CylindricalDetector(const QString& name = defaultName());

    // methods
    //void computeModuleLocations();
    double moduleWidth() const;
    void setAngulationFromFanAngle(uint nbModules, double fanAngle, double radius);
    void setSpacingFromRadius(double radius);
    QVector<double> moduleAngulations() const;
};

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void CylindricalDetector::read(const QJsonObject &json)
{
    AbstractDetector::read(json);

    _angulationPerModule = json.value("angulation per module").toDouble();
    _moduleSpacing = json.value("module spacing").toDouble();
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
inline void CylindricalDetector::write(QJsonObject &json) const
{
    AbstractDetector::write(json);

    json.insert("angulation per module", _angulationPerModule);
    json.insert("module spacing", _moduleSpacing);
}

/*!
 * Returns the angulation of module \a module (in radians) with respect to the center of the
 * detector.
 */
inline double CylindricalDetector::angulationOfModule(uint module) const
{
    Q_ASSERT(module < nbDetectorModules());
    return moduleAngulations().at(module);
}

inline SystemComponent* CylindricalDetector::clone() const { return new CylindricalDetector(*this); }

/*!
 * Returns the cone angle of the detector.
 *
 * The cone angle is computed under the assumption that a point source located at the distance of
 * the curvature radius is used.
 */
inline double CylindricalDetector::coneAngle() const
{
    return 2.0 * tan(0.5 * rowCoverage() / curvatureRadius());
}

/*!
 * Returns the total fan angle covered by the detector.
 *
 * This contains the angle between all modules and the fan angle of one isolated module (two times
 * half a module at the borders of the detector):
 * \f$\mathtt{fanAngle}=\left(\mathtt{nbModule}-1\right)\cdot\alpha+2\cdot\arctan(\mathtt{moduleWidth}/
 * (2\cdot\mathtt{radius})\f$
 */
inline double CylindricalDetector::fanAngle() const
{
    return static_cast<double>(nbDetectorModules() - 1) * _angulationPerModule
        + 2.0 * atan(0.5 * moduleWidth() / curvatureRadius());
}

/*!
 * Returns the width (in mm) of an individual module. Computes as number of pixels times width of a
 * pixel.
 */
inline double CylindricalDetector::moduleWidth() const
{
    return static_cast<double>(_nbPixelPerModule.width()) * _pixelDimensions.width();
}

/*!
 * Returns the total coverage (in mm) by the rows of the detector. This is computed as number of
 * rows times height of a module.
 */
inline double CylindricalDetector::rowCoverage() const
{
    return static_cast<double>(_nbPixelPerModule.height()) * _pixelDimensions.height();
}

/*!
 * Sets the module angulations based on the parameters \a nbModules, \a fanAngle, and \a radius.
 *
 * To do so, the required angulation per module \f$\alpha\f$ is computed as:
 * \f$\alpha=\left(\mathtt{fanAngle}-2\cdot\arctan(\mathtt{moduleWidth}/(2\cdot\mathtt{radius})
 * \right)/\left(\mathtt{nbModule}-1\right).\f$
 */
inline void CylindricalDetector::setAngulationFromFanAngle(uint nbModules, double fanAngle, double radius)
{
    _angulationPerModule = (fanAngle - 2.0 * atan(0.5 * moduleWidth() / radius))
        / static_cast<double>(nbModules - 1);
}

/*!
 * Sets the module spacing based on the \a radius.
 */
inline void CylindricalDetector::setSpacingFromRadius(double radius)
{
    _moduleSpacing = radius * sqrt(2.0 * (1.0 - cos(_angulationPerModule)))
        - moduleWidth() * cos(0.5 * _angulationPerModule);
}

} // namespace CTL

/*! \file */

#endif // CYLINDRICALDETECTOR_H
