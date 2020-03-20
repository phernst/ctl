#ifndef CTL_GENERICDETECTOR_H
#define CTL_GENERICDETECTOR_H

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
    CTL_TYPE_ID(101)

    // implementation of abstract interface
    public: QVector<ModuleLocation> moduleLocations() const override;

public:
    GenericDetector(const QSize& nbPixelPerModule,
                    uint nbModules,
                    const QString& name = defaultName());
    GenericDetector(const QSize& nbPixelPerModule,
                    const QSizeF& pixelDimensions,
                    QVector<ModuleLocation> moduleLocations,
                    const QString& name = defaultName());

    // virtual methods
    SystemComponent* clone() const override;
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // setter methods
    void setModuleLocations(QVector<ModuleLocation> moduleLocations);
    void setPixelSize(const QSizeF& size);
    void setSkewCoefficient(double skewCoefficient);

    // static methods
    static QString defaultName();


protected:
    QVector<ModuleLocation> _moduleLocations; //!< Vector of locations of all detector modules.

private:
    GenericDetector(const QString& name = defaultName());
};

} // namespace CTL

/*! \file */

#endif // CTL_GENERICDETECTOR_H
