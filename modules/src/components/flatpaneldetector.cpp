#include "flatpaneldetector.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(FlatPanelDetector)

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

// use documentation of GenericComponent::clone()
SystemComponent* FlatPanelDetector::clone() const { return new FlatPanelDetector(*this); }


QVector<AbstractDetector::ModuleLocation> FlatPanelDetector::moduleLocations() const
{
    return QVector<ModuleLocation>(1);
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
void FlatPanelDetector::fromVariant(const QVariant& variant)
{
    AbstractDetector::fromVariant(variant);
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
QVariant FlatPanelDetector::toVariant() const
{
    return AbstractDetector::toVariant();
}

/*!
 * Returns the location of the detector module.
 *
 * Convenience method: same as moduleLocations().first().
 */
AbstractDetector::ModuleLocation FlatPanelDetector::location() const
{
    return ModuleLocation();
}

/*!
 * Returns the number of pixels in the detector module.
 *
 * Convenience method: same as nbPixelPerModule().
 */
const QSize& FlatPanelDetector::nbPixels() const { return _nbPixelPerModule; }

const QSizeF FlatPanelDetector::panelDimensions() const
{
    return QSizeF(double(_nbPixelPerModule.width())*_pixelDimensions.width(),
                  double(_nbPixelPerModule.height())*_pixelDimensions.height());
}

} // namespace CTL
