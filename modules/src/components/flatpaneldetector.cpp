#include "flatpaneldetector.h"

namespace CTL {

FlatPanelDetector::FlatPanelDetector(const QJsonObject &json)
    : AbstractDetector(defaultName())
{
    FlatPanelDetector::read(json);
}

/*!
 * Constructs a FlatPanelDetector with \a nbPixels pixels which have a dimension specified by
 * \a pixelDimensions.
 *
 * \sa GenericDetector::GenericDetector().
 */
FlatPanelDetector::FlatPanelDetector(const QSize& nbPixels,
                                     const QSizeF& pixelDimensions,
                                     const QString& name)
    : AbstractDetector(nbPixels, pixelDimensions, name)
{
}

/*!
 *  Returns a formatted string with information about the object.
 */
QString FlatPanelDetector::info() const
{
    QString ret(AbstractDetector::info());

    ret += typeInfoString(typeid(this));
    ret += (this->type() == FlatPanelDetector::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Returns the default name for the component: "Flat panel detector".
 */
QString FlatPanelDetector::defaultName()
{
    static const QString defName(QStringLiteral("Flat panel detector"));
    static uint counter = 0;
    return counter++ ? defName + " (" + QString::number(counter) + ")" : defName;
}

}
