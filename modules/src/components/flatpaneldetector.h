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
    CTL_TYPE_ID(120)

    // implementation of abstract interface
    public: QVector<ModuleLocation> moduleLocations() const override;

public:
    FlatPanelDetector(const QSize& nbPixels,
                      const QSizeF& pixelDimensions,
                      const QString& name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter methods
    ModuleLocation location() const;
    const QSize& nbPixels() const;

    // other methods
    QSizeF panelDimensions() const;

    // static methods
    static QString defaultName();

private:
    FlatPanelDetector() = default;
};

} // namespace CTL

/*! \file */

#endif // FLATPANELDETECTOR_H
