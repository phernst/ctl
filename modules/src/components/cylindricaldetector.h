#ifndef CTL_CYLINDRICALDETECTOR_H
#define CTL_CYLINDRICALDETECTOR_H

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
    CTL_TYPE_ID(110)

    // implementation of abstract interface
    public: QVector<ModuleLocation> moduleLocations() const override;

public:
    CylindricalDetector(const QSize& nbPixelPerModule,
                        const QSizeF& pixelDimensions,
                        uint nbDetectorModules,
                        double angulationPerModule,
                        double moduleSpacing,
                        const QString& name = defaultName());

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
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter methods
    double angulationOfModule(uint module) const;
    double moduleSpacing() const;

    // other methods
    double coneAngle() const;
    double curvatureRadius() const;
    double fanAngle() const;
    double rowCoverage() const;

    // static methods
    static QString defaultName();

protected:
    uint _nbModules; //!< Number of individual (flat-panel) modules in the detector.
    double _angulationPerModule; //!< Angulation (in rad) between adjacent detector modules.
    double _moduleSpacing; //!< Gap between adjacent detector modules (zero if fitting tight).

private:
    CylindricalDetector(const QString& name = defaultName());

    // methods
    //void computeModuleLocations();
    double moduleWidth() const;
    void setAngulationFromFanAngle(uint nbModules, double fanAngle, double radius);
    void setSpacingFromRadius(double radius);
    QVector<double> moduleAngulations() const;
};

} // namespace CTL

/*! \file */

#endif // CTL_CYLINDRICALDETECTOR_H
