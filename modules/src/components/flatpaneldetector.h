#ifndef FLATPANELDETECTOR_H
#define FLATPANELDETECTOR_H

#include "abstractdetector.h"
#include <QSize>

namespace CTL {
/*!
 * \class FlatPanelDetector
 *
 * \brief Specialized sub-class of AbstractDetector for flat panel detectors.
 *
 * Contrary to the AbstractDetector class, this class is intended to hold only a single detector
 * module.
 */
class FlatPanelDetector : public AbstractDetector
{
    ADD_TO_COMPONENT_ENUM(120)

    // implementation of abstract interface
    public: QVector<ModuleLocation> moduleLocations() const override;

public:
    FlatPanelDetector(const QSize& nbPixels,
                      const QSizeF& pixelDimensions,
                      const QString& name = defaultName());
    FlatPanelDetector(const QJsonObject& json);

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // getter methods
    ModuleLocation location() const;
    const QSize& nbPixels() const;

    // other methods
    const QSizeF panelDimensions() const;

    // static methods
    static QString defaultName();

};

// use documentation of GenericComponent::clone()
inline SystemComponent* FlatPanelDetector::clone() const { return new FlatPanelDetector(*this); }


inline QVector<AbstractDetector::ModuleLocation> FlatPanelDetector::moduleLocations() const
{
    return QVector<ModuleLocation>(1);
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void FlatPanelDetector::read(const QJsonObject &json)
{
    AbstractDetector::read(json);
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
inline void FlatPanelDetector::write(QJsonObject &json) const
{
    AbstractDetector::write(json);
}

/*!
 * Returns the location of the detector module.
 *
 * Convenience method: same as moduleLocations().first().
 */
inline AbstractDetector::ModuleLocation FlatPanelDetector::location() const
{
    return ModuleLocation();
}

/*!
 * Returns the number of pixels in the detector module.
 *
 * Convenience method: same as nbPixelPerModule().
 */
inline const QSize& FlatPanelDetector::nbPixels() const { return _nbPixelPerModule; }

inline const QSizeF FlatPanelDetector::panelDimensions() const
{
    return QSizeF(double(_nbPixelPerModule.width())*_pixelDimensions.width(),
                  double(_nbPixelPerModule.height())*_pixelDimensions.height());
}

} // namespace CTL

/*! \file */

#endif // FLATPANELDETECTOR_H
