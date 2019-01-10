#include "genericdetector.h"

namespace CTL {

GenericDetector::GenericDetector(const QJsonObject& json)
    : AbstractDetector(defaultName())
{
    GenericDetector::read(json);
}

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

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
void GenericDetector::read(const QJsonObject &json)
{
    AbstractDetector::read(json);

    _moduleLocations.clear();
    QJsonArray locations = json.value("module locations").toArray();
    for(const auto& obj : locations)
    {
        auto moduleObj = obj.toObject();

        mat::Location loc;
        loc.fromVariant(moduleObj.toVariantMap());

        _moduleLocations.append(loc);
    }

}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
void GenericDetector::write(QJsonObject &json) const
{
    AbstractDetector::write(json);

    QJsonArray modLocs;
    for(const auto& mod : _moduleLocations)
        modLocs.append(QJsonValue::fromVariant(mod.toVariant()));

    json.insert("module locations", modLocs);
}

} // namespace CTL
