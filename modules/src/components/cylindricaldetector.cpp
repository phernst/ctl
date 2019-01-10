#include "cylindricaldetector.h"

#include <QSize>
#include <QSizeF>
#include <QtMath>
#include <iostream>
#include <limits>

namespace CTL {

CylindricalDetector::CylindricalDetector(const QJsonObject &json)
    : AbstractDetector(defaultName())
{
    CylindricalDetector::read(json);
    //computeModuleLocations();
}

CylindricalDetector::CylindricalDetector(const QString &name)
    : AbstractDetector(name)
{
}

/*!
 * Constructs a CylindricalDetector object that is composed of \a nbDetectorModules (flat-panel)
 * detector modules, each of which with \a nbPixelPerModule pixels with dimensions \a
 * pixelDimensions. The arrangement of the individual modules is constructed based on the \a
 * angulationPerModule and \a moduleSpacing parameters.
 */
CylindricalDetector::CylindricalDetector(const QSize& nbPixelPerModule,
                                         const QSizeF& pixelDimensions,
                                         uint nbDetectorModules,
                                         double angulationPerModule,
                                         double moduleSpacing,
                                         const QString& name)
    : AbstractDetector(nbPixelPerModule, pixelDimensions, name)
    , _nbModules(nbDetectorModules)
    , _angulationPerModule(angulationPerModule)
    , _moduleSpacing(moduleSpacing)
{
    //computeModuleLocations();
}

/*!
 * Factory method to construct a CylindricalDetector from the parameters \a angulationPerModule and
 * \a moduleSpacing. This method simply calls the constructor.
 *
 * \sa CylindricalDetector().
 */
CylindricalDetector CylindricalDetector::fromAngulationAndSpacing(const QSize& nbPixelPerModule,
                                                                  const QSizeF& pixelDimensions,
                                                                  uint nbDetectorModules,
                                                                  double angulationPerModule,
                                                                  double moduleSpacing,
                                                                  const QString& name)
{
    return CylindricalDetector(nbPixelPerModule, pixelDimensions, nbDetectorModules,
                               angulationPerModule, moduleSpacing, name);
}

/*!
 * Factory method to construct a CylindricalDetector from the parameters \a radius and \a fanAngle
 * instead of module spacing and angulation (as used in the constructor). Use this factory if you
 * want to specify the detector system by its curvature radius and fan angle.
 *
 * Module angulation and spacing are computed using setAngulationFromFanAngle() and
 * setSpacingFromRadius(), respectively.
 */
CylindricalDetector CylindricalDetector::fromRadiusAndFanAngle(const QSize& nbPixelPerModule,
                                                               const QSizeF& pixelDimensions,
                                                               uint nbDetectorModules,
                                                               double radius,
                                                               double fanAngle,
                                                               const QString& name)
{
    CylindricalDetector ret;

    ret.rename(name);

    ret._nbModules = nbDetectorModules;
    ret._nbPixelPerModule = nbPixelPerModule;
    ret._pixelDimensions = pixelDimensions;

    ret.setAngulationFromFanAngle(nbDetectorModules, fanAngle, radius);
    ret.setSpacingFromRadius(radius);

    //ret.computeModuleLocations();

    return ret;
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base classes (GenericDetector and GenericComponent), the
 * info string contains the following details:
 * \li Fan angle
 * \li Cone angle
 * \li Row coverage
 * \li Curvature radius
 */
QString CylindricalDetector::info() const
{
    QString ret(AbstractDetector::info());

    // clang-format off
    ret += typeInfoString(typeid(this));
    ret += "\tRow coverage: "     + QString::number(rowCoverage()) + " mm\n"
           "\tFan angle: "        + QString::number(qRadiansToDegrees(fanAngle())) + " deg\n"
           "\tCone angle: "       + QString::number(qRadiansToDegrees(coneAngle())) + " deg\n"
           "\tCurvature radius: " + QString::number(curvatureRadius()) + " mm\n";
    ret += (this->type() == CylindricalDetector::Type) ? "}\n" : "";
    // clang-format on

    return ret;
}

/*!
 * Returns the curvature radius of the given detector arrangement.
 *
 * This is computed as: \f$r=d/\sqrt{2(1-\cos\alpha)}\f$
 * where \f$r\f$ denotes the curvature radius and \f$\alpha=\mathtt{\_angulationPerModule}\f$. The
 * length \f$d\f$ is computed as follows:
 * \f$d=\mathtt{\_moduleSpacing}+\mathtt{\_moduleWidth}\cdot\cos\left(\alpha/2\right)\f$.
 *
 * Returns std::numeric_limits<double>::max() if the module angulation is zero (flat panel),
 */
double CylindricalDetector::curvatureRadius() const
{
    if(qFuzzyIsNull(_angulationPerModule))
        return std::numeric_limits<double>::max(); // flat detector --> radius becomes infinite

    const double modWidth = moduleWidth();
    double d = _moduleSpacing + modWidth * cos(_angulationPerModule / 2.0);

    return d / sqrt(2.0 * (1.0 - cos(_angulationPerModule)));
}

/*!
 * Returns the default name for the component: "Cylindrical detector".
 */
QString CylindricalDetector::defaultName()
{
    static const QString defName(QStringLiteral("Cylindrical detector"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

/*!
 * This method computes the locations, i.e. their relative position and orientation w.r.t. the
 * center of the full detector, of all detector modules based on the defining parameters `radius`
 * (extracted using curvatureRadius()) and `_moduleAngulations` (set in advance using
 * setEquidistantModuleAngulation()).
 */
QVector<AbstractDetector::ModuleLocation> CylindricalDetector::moduleLocations() const
{
    QVector<ModuleLocation> loc;
    loc.reserve(_nbModules);

    const double radius = curvatureRadius(); // the radius of the cylinder
    // starting point in the middle of the detector (in CT coordinates)
    const Vector3x1 pt({ 0.0, 0.0, radius });

    ModuleLocation tmp;
    Matrix3x3 rotMat;
    Vector3x1 rotPt;

    const auto modAngul = moduleAngulations();

    for(uint mod = 0; mod < _nbModules; ++mod)
    {
        rotMat = mat::rotationMatrix(modAngul.at(mod), Qt::YAxis);
        rotPt = rotMat * pt; // rotate the starting point
        rotPt.get<2>() -= radius; // translate the resulting vector to the detector position

        tmp.position = rotPt;
        tmp.rotation = rotMat.transposed(); // store the passive form

        loc.append(tmp);
    }

    return loc;
}

/*!
 * Computes and returns a vector \f$\vec{\varphi}\f$ with equidistributed angulation values.
 *
 * For a given number of \f$N\f$ modules (extracted using nbDetectorModules()) and an angulation
 * \f$\alpha\f$ between adjacent modules
 * (\a angulationPerModule), this is yields:
 * \f$
 * \vec{\varphi}=\begin{cases}
 * \left(-(N-1)/2\cdot\alpha,-(N-2)/2\cdot\alpha,...,0,...,(N-2)/2\cdot\alpha,(N-1)/2\cdot\alpha
 * \right) & N\,\textrm{odd}\\
 * \left(-(N-1)/2\cdot\alpha,-(N-2)/2\cdot\alpha,...,-1/2\cdot\alpha,1/2\cdot\alpha,...,(N-2)/2
 * \cdot\alpha,(N-1)/2\cdot\alpha\right) & N\,\textrm{even} \end{cases} \f$
 */
QVector<double> CylindricalDetector::moduleAngulations() const
{
    const uint nbModules = _nbModules;
    QVector<double> modAngul(nbModules);
    for(uint module = 0; module < nbModules; ++module)
        modAngul[module] = (double(module) - double(nbModules) * 0.5 + 0.5) * _angulationPerModule;

    return modAngul;
}

} // namespace CTL
