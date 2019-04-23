#include "genericdetector.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(GenericDetector)

/*!
 * Constructs an empty object named \a name.
 */
GenericDetector::GenericDetector(const QString &name)
    : AbstractDetector(name)
{
}

/*!
 * Constructs a generic detector element with modules that have \a nbPixelPerModule pixels
 * (`channels` x `rows`) with dimensions of \a pixelDimensions (`width` x `height`). The arrangement
 * of the individual modules with respect to the entire detector system are specified in \a
 * moduleLocations as a vector of ModuleLocation elements (each of which contain the modules
 * position and rotation).
 */
GenericDetector::GenericDetector(const QSize& nbPixelPerModule,
                                 const QSizeF& pixelDimensions,
                                 QVector<ModuleLocation> moduleLocations,
                                 const QString& name)
    : AbstractDetector(nbPixelPerModule, pixelDimensions, name)
    , _moduleLocations(std::move(moduleLocations))
{
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class, the info string contains the following
 * details: \li Nb. of detector modules \li Nb. of pixels per module \li Pixel dimensions.
 */
QString GenericDetector::info() const
{
    QString ret(AbstractDetector::info());

    // clang-format off
    ret +=
       typeInfoString(typeid(this)) +
       "\tNb. of detector modules: "   + QString::number(nbDetectorModules()) + "\n"
       "\tNb. of pixels per module: "  + QString::number(_nbPixelPerModule.width()) + " x " +
                                         QString::number(_nbPixelPerModule.height()) + "\n"
       "\tPixel dimensions: "          + QString::number(_pixelDimensions.width()) + " mm x " +
                                         QString::number(_pixelDimensions.height()) + " mm\n";

    ret += (this->type() == GenericDetector::Type) ? "}\n" : "";
    // clang-format on

    return ret;
}

/*!
 * Returns the default name for the component: "Generic detector".
 */
QString GenericDetector::defaultName()
{
    static const QString defName(QStringLiteral("Generic detector"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

// Use SerializationInterface::fromVariant() documentation.
void GenericDetector::fromVariant(const QVariant& variant)
{
    AbstractDetector::fromVariant(variant);

    _moduleLocations.clear();
    QVariantMap varMap = variant.toMap();
    auto locations = varMap.value("module locations").toList();
    for(const auto& var : locations)
    {
        mat::Location loc;
        loc.fromVariant(var);

        _moduleLocations.append(loc);
    }

}

// Use SerializationInterface::toVariant() documentation.
QVariant GenericDetector::toVariant() const
{
    QVariantMap ret = AbstractDetector::toVariant().toMap();

    QVariantList modLocs;
    for(const auto& mod : _moduleLocations)
        modLocs.append(mod.toVariant());

    ret.insert("module locations", modLocs);

    return ret;
}

// use documentation of GenericComponent::clone()
SystemComponent* GenericDetector::clone() const { return new GenericDetector(*this); }

/*!
 * Returns the vector that stores the module locations.
 *
 * Each ModuleLocation object contains the position of the module in world coordinates as well as a
 * rotation matrix that represents the transformation from the module's coordinate system to the
 * CT-system (i.e. the coordinate system of the detector as a whole).
 */
QVector<AbstractDetector::ModuleLocation> GenericDetector::moduleLocations() const
{
    return _moduleLocations;
}

/*!
 * Sets the module locations to \a moduleLocations.
 */
void GenericDetector::setModuleLocations(QVector<AbstractDetector::ModuleLocation> moduleLocations)
{
    _moduleLocations = std::move(moduleLocations);
}

/*!
 * Sets the pixel size to \a size.
 */
void GenericDetector::setPixelSize(const QSizeF &size)
{
    _pixelDimensions = size;
}

/*!
 * Sets the skew coefficient to \a skewCoefficient.
 */
void GenericDetector::setSkewCoefficient(double skewCoefficient)
{
    _skewCoefficient = skewCoefficient;
}

} // namespace CTL
