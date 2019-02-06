#ifndef ABSTRACTDETECTOR_H
#define ABSTRACTDETECTOR_H

#include "systemcomponent.h"
#include "mat/matrix_utils.h"   // for definition of mat::Location
#include "img/singleviewdata.h" // for definition of SingleViewData::Dimensions
#include "models/abstractdatamodel.h"

#include <QJsonArray>
#include <QSize>
#include <QVariantMap>

/*
 * NOTE: This is header only.
 */

namespace CTL {
/*!
 * \class AbstractDetector
 *
 * \brief Base class for detector components.
 *
 * This is the base class for all detector components.
 * Detectors are always considered to be composed of multiple flat panel elements, called modules.
 * The arrangement of all modules is described by a vector of ModuleLocation objects, one for each
 * of the modules. The ModuleLocation object must contain the position of the module in world
 * coordinates as well as a rotation matrix that represents the transformation from the module's
 * coordinate system to the CT-system (i.e. the coordinate system of the detector as a whole). In
 * addition to the arrangement, detector modules are characterized by their number of pixels
 * (`channels` x `rows`) and the corresponding dimensions of an individual pixel (`width` x
 * `height`).
 *
 * Custom detector types can be implemented by creating a sub-class of AbstractDetector.
 * Such sub-classes need to implement the method moduleLocations() that should extract the location
 * of all flat panel modules in the detector based on the specific parametrization chosen for the
 * sub-class.
 *
 * When creating a sub-class of AbstractDetector, make sure to register the new component in the
 * enumeration using the #CTL_TYPE_ID(newIndex) macro. It is required to specify a value
 * for \a newIndex that is not already in use. This can be easily achieved by use of values starting
 * from GenericComponent::UserType, as these are reserved for user-defined types.
 *
 * To provide full compatibility within existing functionality, it is recommended to reimplement the
 * read() and write() method, such that these cover newly introduced information of the sub-class.
 * The new class should then also be added to switch-case list inside the implementation of
 * parseComponentFromJson(const QJsonObject&) found in the header file "components/jsonparser.h".
 */
class AbstractDetector : public SystemComponent
{
    CTL_TYPE_ID(100)
    DECLARE_ELEMENTAL_TYPE

public:
    typedef mat::Location ModuleLocation;
    enum SaturationModelType { Extinction, Intensity, Undefined };

    // abstract interface
    public:virtual QVector<ModuleLocation> moduleLocations() const = 0;

public:
    AbstractDetector(const QSize& nbPixelPerModule,
                    const QSizeF& pixelDimensions,
                    const QString& name);

    // virtual methods
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // deprecated
    void read(const QJsonObject& json) override;  // JSON
    void write(QJsonObject& json) const override; // JSON

    // setter methods
    void setSaturationModel(AbstractDataModel* model, SaturationModelType type);
    void setSaturationModel(std::unique_ptr<AbstractDataModel> model, SaturationModelType type);

    // getter methods
    uint nbDetectorModules() const;
    const QSize& nbPixelPerModule() const;
    const QSizeF& pixelDimensions() const;
    ModuleLocation moduleLocation(uint module) const;
    const AbstractDataModel* saturationModel() const;
    SaturationModelType saturationModelType() const;

    // other methods
    bool hasSaturationModel() const;
    QSizeF moduleDimensions() const;
    SingleViewData::Dimensions viewDimensions() const;

protected:
    AbstractDetector() = default;
    AbstractDetector(const QString& name);

    QSize _nbPixelPerModule;
    QSizeF _pixelDimensions;

    DataModelPtr _saturationModel;
    SaturationModelType _saturationModelType = Undefined;
};

/*!
 * Returns the number of detector modules.
 *
 * Same as moduleLocations().size().
 */
inline uint AbstractDetector::nbDetectorModules() const { return moduleLocations().count(); }

/*!
 * Returns the number of pixels in an individual module as QSize. Dimensions are specified as
 * detector `channels` x `rows`.
 */
inline const QSize& AbstractDetector::nbPixelPerModule() const { return _nbPixelPerModule; }

/*!
 * Returns the dimensions of an individual pixel as QSizeF. Dimensions are specified as `width` x
 * `height` (or x-spacing and z-spacing, respectively).
 */
inline const QSizeF& AbstractDetector::pixelDimensions() const { return _pixelDimensions; }

/*!
 * Returns the (physical) dimensions of an individual detector module as QSizeF. Dimensions are
 * specified as `width` x `height`.
 */
inline QSizeF AbstractDetector::moduleDimensions() const
{
    return { _nbPixelPerModule.width()*_pixelDimensions.width(),
                _nbPixelPerModule.height()*_pixelDimensions.height() };
}

/*!
 * Returns the dimensions of a single view that would be acquired by this instance. This contains
 * number of channels (per module), number of rows (per module) and number of modules.
 */
inline SingleViewData::Dimensions AbstractDetector::viewDimensions() const
{
    SingleViewData::Dimensions ret;
    ret.nbChannels = _pixelDimensions.width();
    ret.nbRows     = _pixelDimensions.height();
    ret.nbModules  = nbDetectorModules();
    return ret;
}

/*!
 * Returns the location of module \a module. Same as moduleLocations().at(module).
 *
 * Using this method is typically very inefficient, as it always requires computation of all module
 * locations. In case you need multiple calls to this method, consider storing a local copy of the
 * entire set of locations (using moduleLocations()) and querying individual module locations from
 * that local copy using QVector::at().
 *
 * \sa moduleLocations()
 */
inline AbstractDetector::ModuleLocation AbstractDetector::moduleLocation(uint module) const
{
    Q_ASSERT(module < nbDetectorModules());
    return moduleLocations().at(module);
}

/*!
 * Returns a pointer to the saturation model of this instance.
 */
inline const AbstractDataModel* AbstractDetector::saturationModel() const
{
    return _saturationModel.get();
}

/*!
 * Returns the type of the saturation model, i.e. whether it refers to extinction or intensity
 * values.
 */
inline AbstractDetector::SaturationModelType AbstractDetector::saturationModelType() const
{
    return _saturationModelType;
}

/*!
 * Returns true if this instance has a saturation model.
 */
inline bool AbstractDetector::hasSaturationModel() const
{
    return (_saturationModel.ptr != nullptr);
}

/*!
 * Constructs an empty object named \a name.
 */
inline AbstractDetector::AbstractDetector(const QString &name)
    : SystemComponent(name)
{
}

/*!
 * Constructs an AbstractDetector object with name \a name which contains modules that have
 * \a nbPixelPerModule pixels (`channels` x `rows`) with dimensions of \a pixelDimensions
 * (`width` x `height`).
 */
inline AbstractDetector::AbstractDetector(const QSize& nbPixelPerModule,
                                 const QSizeF& pixelDimensions,
                                 const QString& name)
    : SystemComponent(name)
    , _nbPixelPerModule(nbPixelPerModule)
    , _pixelDimensions(pixelDimensions)
{
}

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class, the info string contains the following
 * details: \li Nb. of detector modules \li Nb. of pixels per module \li Pixel dimensions.
 */
inline QString AbstractDetector::info() const
{
    QString ret(SystemComponent::info());

    // clang-format off
    ret +=
       typeInfoString(typeid(this)) +
       "\tNb. of modules: "            + QString::number(nbDetectorModules()) + "\n"
       "\tNb. of pixels per module: "  + QString::number(_nbPixelPerModule.width()) + " x " +
                                         QString::number(_nbPixelPerModule.height()) + "\n"
       "\tPixel dimensions: "          + QString::number(_pixelDimensions.width()) + " mm x " +
                                         QString::number(_pixelDimensions.height()) + " mm\n";

    ret += (this->type() == AbstractDetector::Type) ? "}\n" : "";
    // clang-format on

    return ret;
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void AbstractDetector::read(const QJsonObject &json)
{
    SystemComponent::read(json);

    QJsonObject nbPixels = json.value("pixel per module").toObject();
    _nbPixelPerModule.setWidth(nbPixels.value("channels").toInt());
    _nbPixelPerModule.setHeight(nbPixels.value("rows").toInt());

    QJsonObject pixelDim = json.value("pixel dimensions").toObject();
    _pixelDimensions.setWidth(pixelDim.value("width").toDouble());
    _pixelDimensions.setHeight(pixelDim.value("height").toDouble());
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
inline void AbstractDetector::write(QJsonObject &json) const
{
    SystemComponent::write(json);

    QJsonObject nbPixels;
    nbPixels.insert("channels",_nbPixelPerModule.width());
    nbPixels.insert("rows", _nbPixelPerModule.height());

    QJsonObject pixelDim;
    pixelDim.insert("width",_pixelDimensions.width());
    pixelDim.insert("height", _pixelDimensions.height());

    json.insert("pixel per module", nbPixels);
    json.insert("pixel dimensions", pixelDim);
}

/*!
 * Reads all member variables from the QVariant \a variant.
 */
inline void AbstractDetector::fromVariant(const QVariant& variant)
{
    SystemComponent::fromVariant(variant);

    QVariantMap varMap = variant.toMap();

    auto nbPixels = varMap.value("pixel per module").toMap();
    _nbPixelPerModule.setWidth(nbPixels.value("channels").toInt());
    _nbPixelPerModule.setHeight(nbPixels.value("rows").toInt());

    auto pixelDim = varMap.value("pixel dimensions").toMap();
    _pixelDimensions.setWidth(pixelDim.value("width").toDouble());
    _pixelDimensions.setHeight(pixelDim.value("height").toDouble());

    QVariant saturationModel = varMap.value("saturation model");
    _saturationModel.ptr.reset(JsonSerializer::parseDataModel(saturationModel));

    int satModTypeVal = varMap.value("saturation model type").toMap().value("enum value").toInt();
    _saturationModelType = SaturationModelType(satModTypeVal);
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
inline QVariant AbstractDetector::toVariant() const
{
    QVariantMap ret = SystemComponent::toVariant().toMap();

    QVariantMap nbPixels;
    nbPixels.insert("channels",_nbPixelPerModule.width());
    nbPixels.insert("rows", _nbPixelPerModule.height());

    QVariantMap pixelDim;
    pixelDim.insert("width",_pixelDimensions.width());
    pixelDim.insert("height", _pixelDimensions.height());

    QVariant saturationModel = hasSaturationModel() ? _saturationModel.ptr->toVariant()
                                                    : QVariant();

    QVariantMap satModelType;
    satModelType.insert("enum value", _saturationModelType);
    satModelType.insert("meaning", "0: Extinction, 1: Intensity, 2: Undefined");

    ret.insert("pixel per module", nbPixels);
    ret.insert("pixel dimensions", pixelDim);
    ret.insert("saturation model", saturationModel);
    ret.insert("saturation model type", satModelType);

    return ret;
}

/*!
 * Sets the saturation model to \a model. The argument \a type must specify whether the passed model
 * refers to extinction values or intensities.
 */
inline void AbstractDetector::setSaturationModel(AbstractDataModel *model, AbstractDetector::SaturationModelType type)
{
    _saturationModel.reset(model);
    _saturationModelType = type;
}

/*!
 * Sets the saturation model to \a model. The argument \a type must specify whether the passed model
 * refers to extinction values or intensities.
 */
inline void AbstractDetector::setSaturationModel(std::unique_ptr<AbstractDataModel> model,
                                          SaturationModelType type)
{
    _saturationModel.ptr = std::move(model);
    _saturationModelType = type;
}

/*!
 * \fn virtual int AbstractDetector::genericType() const
 *
 * Returns the type id of the AbstractDetector class.
 *
 * This method can be used to determine whether the base type of an object is AbstractDetector.
 */

/*!
 * \typedef AbstractDetector::ModuleLocation
 *
 * Synonym for mat::Location.
 */

} // namespace CTL

/*! \file */
///@{
/*!
 * \fn std::unique_ptr<AbstractDetector> CTL::makeDetector(ConstructorArguments&&... arguments)
 * \relates AbstractDetector
 *
 * Factory method to construct an object of any sub-class of AbstractDetector and wrap the object in
 * an std::unique_ptr<AbstractDetector>.
 *
 * This is similar to the more general method GenericComponent::makeComponent() with the difference
 * that it returns a unique pointer to the AbstractDetector base type instead of GenericComponent.
 *
 * \sa GenericComponent::makeComponent().
 */
///@}

#endif // ABSTRACTDETECTOR_H
