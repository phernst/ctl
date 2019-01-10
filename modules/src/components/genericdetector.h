#ifndef GENERICDETECTOR_H
#define GENERICDETECTOR_H

#include "abstractdetector.h"
#include "mat/matrix_utils.h"

namespace CTL {
/*!
 * \class GenericDetector
 *
 * \brief Generic implementation of a detector component.
 *
 * This class implements AbstractDetector with a straightforward parametrization of the locations
 * of individual flat panel detector modules. In particular, for each module, the location (i.e.
 * position and rotation information) is stored in a private member of this class.
 *
 * The required interface method moduleLocations() then simply returns this private member.
 *
 * Use setModuleLocations() to change the location of the modules.
 *
 */
class GenericDetector : public AbstractDetector
{
    ADD_TO_COMPONENT_ENUM(101)

    // implementation of abstract interface
    public: QVector<ModuleLocation> moduleLocations() const override;

public:
    GenericDetector(const QString& name = defaultName());
    GenericDetector(const QSize& nbPixelPerModule,
                    const QSizeF& pixelDimensions,
                    QVector<ModuleLocation> moduleLocations,
                    const QString& name = defaultName());
    GenericDetector(const QJsonObject& json);

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void read(const QJsonObject& json) override;     // JSON
    void write(QJsonObject& json) const override;    // JSON

    // setter methods
    void setModuleLocations(QVector<ModuleLocation> moduleLocations);

    // static methods
    static QString defaultName();


protected:
    QVector<ModuleLocation> _moduleLocations;
};

// use documentation of GenericComponent::clone()
inline SystemComponent* GenericDetector::clone() const { return new GenericDetector(*this); }

/*!
 * Returns the vector that stores the module locations.
 *
 * Each ModuleLocation object contains the position of the module in world coordinates as well as a
 * rotation matrix that represents the transformation from the module's coordinate system to the
 * CT-system (i.e. the coordinate system of the detector as a whole).
 */
inline QVector<AbstractDetector::ModuleLocation> GenericDetector::moduleLocations() const
{
    return _moduleLocations;
}

/*!
 * Sets the module locations to \a moduleLocations.
 */
inline void GenericDetector::setModuleLocations(QVector<AbstractDetector::ModuleLocation> moduleLocations)
{
    _moduleLocations = std::move(moduleLocations);
}

} // namespace CTL

/*! \file */

#endif // GENERICDETECTOR_H
