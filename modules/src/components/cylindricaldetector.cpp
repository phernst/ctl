#include "cylindricaldetector.h"

#include <QSize>
#include <QSizeF>
#include <QtMath>
#include <iostream>
#include <limits>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(CylindricalDetector)

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

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void CylindricalDetector::fromVariant(const QVariant& variant)
{
    AbstractDetector::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    _angulationPerModule = varMap.value("angulation per module").toDouble();
    _moduleSpacing = varMap.value("module spacing").toDouble();
    _nbModules = varMap.value("number of modules").toUInt();
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant CylindricalDetector::toVariant() const
{
    QVariantMap ret = AbstractDetector::toVariant().toMap();

    ret.insert("angulation per module", _angulationPerModule);
    ret.insert("module spacing", _moduleSpacing);
    ret.insert("number of modules", _nbModules);

    return ret;
}

/*!
 * Returns the angulation of module \a module (in radians) with respect to the center of the
 * detector.
 */
double CylindricalDetector::angulationOfModule(uint module) const
{
    Q_ASSERT(module < nbDetectorModules());
    return moduleAngulations().at(module);
}

SystemComponent* CylindricalDetector::clone() const { return new CylindricalDetector(*this); }

/*!
 * Returns the cone angle of the detector.
 *
 * The cone angle is computed under the assumption that a point source located at the distance of
 * the curvature radius is used.
 */
double CylindricalDetector::coneAngle() const
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
double CylindricalDetector::fanAngle() const
{
    return static_cast<double>(nbDetectorModules() - 1) * _angulationPerModule
        + 2.0 * atan(0.5 * moduleWidth() / curvatureRadius());
}

/*!
 * Returns the width (in mm) of an individual module. Computes as number of pixels times width of a
 * pixel.
 */
double CylindricalDetector::moduleWidth() const
{
    return static_cast<double>(_nbPixelPerModule.width()) * _pixelDimensions.width();
}

/*!
 * Returns the total coverage (in mm) by the rows of the detector. This is computed as number of
 * rows times height of a module.
 */
double CylindricalDetector::rowCoverage() const
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
void CylindricalDetector::setAngulationFromFanAngle(uint nbModules, double fanAngle, double radius)
{
    _angulationPerModule = (fanAngle - 2.0 * atan(0.5 * moduleWidth() / radius))
        / static_cast<double>(nbModules - 1);
}

/*!
 * Sets the module spacing based on the \a radius.
 */
void CylindricalDetector::setSpacingFromRadius(double radius)
{
    _moduleSpacing = radius * sqrt(2.0 * (1.0 - cos(_angulationPerModule)))
        - moduleWidth() * cos(0.5 * _angulationPerModule);
}

} // namespace CTL
