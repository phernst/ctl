#ifndef ABSTRACTSOURCE_H
#define ABSTRACTSOURCE_H
#include "mat/matrix_types.h"
#include "models/intervaldataseries.h"
#include "models/xrayspectrummodels.h"
#include "systemcomponent.h"
#include <QJsonArray>
#include <QSizeF>

/*
 * NOTE: This is header only.
 */

namespace CTL {

/*!
 * \class AbstractSource
 *
 * \brief Base class for source components.
 *
 * This is the base class for all source components. A source component is characterized by the size
 * and position (optional) of its focal spot (i.e. the area from which radiation is emitted), the
 * emitted X-ray spectrum (relative intensity contribution for each photon energy) and the overall
 * photon flux that is emitted by the source.
 *
 * The AbstractSource class has two abstract interface methods: spectrum() and nominalPhotonFlux().
 * The spectrum() method must provide a sampled representation of the current radiation spectrum
 * emitted by the source component. Spectrum computation may be based on an AbstractSpectralModel,
 * which can be set using the corresponding setter method setSpectrumModel().
 * The nominalPhotonFlux() method shall return the (unmodified) flux of photons emitted by the
 * source. This is used whithin the actual getter method photonFlux(), which returns the nominal
 * flux multiplied by an (optional, defaults to 1.0) modifier (see fluxModifier() and
 * setFluxModifier()).
 *
 * Definition of the focal spot geometry, i.e. both size and position, refers to CT coordinates. The
 * focal spot size is specified in the x-y-plane. Hence, a focal spot with size QSizeF(1.0, 2.0)
 * would, for example, have an extension of 1mm in x-direction and 2mm in y-direction. The position
 * of the focal spot can be defined in all three directions (in the CT coordinate system). This can
 * be used to describe shifted focal spot positions. The zero position (i.e. Vector3x1(0.0, 0.0,
 * 0.0)) corresponds to an unshifted focal spot.
 * Example: A source with focal spot position Vector3x1(0.0, 10.0, -50.0), would have its focal spot
 * shifted by 10mm in y-direction and 50mm in negative z-direction (the latter would, for example,
 * mean that the point of radiation emission is 50mm further away from the detector compared to an
 * unshifed focal spot).
 *
 * When creating a sub-class of AbstractSource, make sure to register the new component in the
 * enumeration using the #CTL_TYPE_ID(newIndex) macro. It is required to specify a value
 * for \a newIndex that is not already in use. This can be easily achieved by use of values starting
 * from GenericComponent::UserType, as these are reserved for user-defined types.
 *
 * To provide full compatibility within existing functionality, it is recommended to reimplement the
 * read() and write() method, such that these cover newly introduced information of the sub-class.
 * The new class should then also be added to switch-case list inside the implementation of
 * parseComponentFromJson(const QJsonObject&) found in the header file "components/jsonparser.h".
 */
class AbstractSource : public SystemComponent
{
    CTL_TYPE_ID(300)
    DECLARE_ELEMENTAL_TYPE

    // abstract interface
    public:virtual IntervalDataSeries spectrum(float from, float to, uint nbSamples) const = 0;
    protected:virtual double nominalPhotonFlux() const = 0;

public:
    AbstractSource(const QString& name);
    AbstractSource(const QSizeF& focalSpotSize, const QString& name);
    AbstractSource(const QSizeF& focalSpotSize,
                   const Vector3x1& focalSpotPosition,
                   const QString& name);
    AbstractSource(const QSizeF& focalSpotSize,
                   const Vector3x1& focalSpotPosition,
                   AbstractXraySpectrumModel* spectumModel,
                   const QString& name);

    // virtual methods
    virtual void setSpectrumModel(AbstractXraySpectrumModel* model);
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // deprecated
    void read(const QJsonObject& json) override; // JSON
    void write(QJsonObject& json) const override; // JSON

    // getter methods
    double photonFlux() const;
    double fluxModifier() const;
    const QSizeF& focalSpotSize() const;
    const Vector3x1& focalSpotPosition() const;
    const AbstractXraySpectrumModel* spectrumModel() const;

    // setter methods
    void setFluxModifier(double modifier);
    void setFocalSpotSize(const QSizeF& size);
    void setFocalSpotSize(double width, double height); // convenience alternative
    void setFocalSpotPosition(const Vector3x1& position);
    void setFocalSpotPosition(double x, double y, double z); // convenience alternative

    // other methods
    bool hasSpectrumModel() const;
    void setSpectrumModel(std::unique_ptr<AbstractXraySpectrumModel> model);

protected:
    QSizeF _focalSpotSize = QSizeF(0.0, 0.0);
    Vector3x1 _focalSpotPosition = Vector3x1(0.0);
    double _fluxModifier = 1.0;

    DataModelPtr _spectrumModel;
};

/*!
 * \fn AbstractSource::nominalPhotonFlux() const
 *
 * Abstract interface method. Returns the nominal photon flux (i.e. photons per cm²) of the source
 * in a distance of one meter.
 *
 * To query the actual photon flux of the source component, use photonFlux().
 */

/*!
 * \fn AbstractSource::spectrum(float from, float to, uint nbSamples) const
 *
 * Abstract interface method. Returns a sampled representation of the current radiation spectrum
 * emitted by the source component. The returned spectrum has \a nbSamples bins covering the energy
 * range of [\a from, \a to] keV. Each energy bin is defined to represent the integral over the
 * contribution of all energies within the bin to the total intensity. The spectrum provides
 * relative intensities, i.e. the sum over all bins equals to one. To
 *
 * Spectrum computation may be based on an AbstractSpectralModel,
 * which can be set using the corresponding setter method setSpectrumModel().
 */

/*!
 * Constructs an AbstractSource object named \a name.
 *
 * Focal spot size defaults to QSizeF(0.0, 0.0) and the focal spot position is initialized with
 * Vector3x1(0.0).
 */
inline AbstractSource::AbstractSource(const QString& name)
    : SystemComponent(name)
{
}

/*!
 * Constructs an AbstractSource object named \a name with a focal spot size of \a focalSpotSize.
 *
 * The focal spot position is initialized with Vector3x1(0.0).
 */
inline AbstractSource::AbstractSource(const QSizeF& focalSpotSize, const QString& name)
    : SystemComponent(name)
    , _focalSpotSize(focalSpotSize)
{
}

/*!
 * Constructs an AbstractSource object named \a name with a focal spot size of \a focalSpotSize and
 * its focal spot position at \a focalSpotPosition.
 */
inline AbstractSource::AbstractSource(const QSizeF& focalSpotSize,
                                      const Vector3x1& focalSpotPosition,
                                      const QString& name)
    : SystemComponent(name)
    , _focalSpotSize(focalSpotSize)
    , _focalSpotPosition(focalSpotPosition)
{
}

/*!
 * Constructs an AbstractSource object named \a name with a focal spot size of \a focalSpotSize and
 * its focal spot position at \a focalSpotPosition. This constructor also sets the spectrum model to
 * \a spectrumModel. This instance manages ownership of \a spectrumModel as a std::shared_ptr.
 */
inline AbstractSource::AbstractSource(const QSizeF& focalSpotSize,
                                      const Vector3x1& focalSpotPosition,
                                      AbstractXraySpectrumModel* spectumModel,
                                      const QString& name)
    : SystemComponent(name)
    , _focalSpotSize(focalSpotSize)
    , _focalSpotPosition(focalSpotPosition)
    , _spectrumModel(std::unique_ptr<AbstractDataModel>(spectumModel))
{
}

/*!
 * Returns the focal spot size of this instance.
 *
 * Definition of the focal spot size is in CT coordinates. In particular, it is specified as the
 * dimension in the x-y-plane.
 */
inline const QSizeF& AbstractSource::focalSpotSize() const { return _focalSpotSize; }

/*!
 * Returns the focal spot position of this instance.
 *
 * Definition of the focal spot position is in CT coordinates.
 */
inline const Vector3x1& AbstractSource::focalSpotPosition() const { return _focalSpotPosition; }

/*!
 * Returns a pointer to the spectrum model of this instance.
 */
inline const AbstractXraySpectrumModel *AbstractSource::spectrumModel() const
{
    return static_cast<AbstractXraySpectrumModel*>(_spectrumModel.ptr.get());
}

/*!
 * Sets the focal spot size \a size.
 *
 * Definition of the focal spot size is in CT coordinates. In particular, it is specified as the
 * dimension in the x-y-plane.
 */
inline void AbstractSource::setFocalSpotSize(const QSizeF& size) { _focalSpotSize = size; }

/*!
 * Sets the focal spot size to a rectangle with dimensions \a width x \a height.
 *
 * Convenience alternative. Same as setFocalSpotSize(QSizeF(\a width, \a height)).
 */
inline void AbstractSource::setFocalSpotSize(double width, double height)
{
    _focalSpotSize = QSizeF(width, height);
}

/*!
 * Sets the focal spot position to \a position.
 *
 * Definition of the focal spot position is in CT coordinates.
 */
inline void AbstractSource::setFocalSpotPosition(const Vector3x1& position)
{
    _focalSpotPosition = position;
}

/*!
 * Sets the focal spot position to the point (\a x, \a y, \a z).
 *
 * Convenience alternative. Same as setFocalSpotPosition(Vector3x1({ \a x, \a y, \a z })).
 */
inline void AbstractSource::setFocalSpotPosition(double x, double y, double z)
{
    _focalSpotPosition = Vector3x1({ x, y, z });
}

/*!
 * Returns true if a spectrum model is available in this instance.
 *
 * \sa setSpectrumModel().
 */
inline bool AbstractSource::hasSpectrumModel() const { return (_spectrumModel.ptr != nullptr); }

/*!
 * Returns a formatted string with information about the object.
 *
 * In addition to the information from the base class, the info string contains the following
 * details: \li Focal spot size \li Focal spot position \li Flux modifier.
 */
inline QString AbstractSource::info() const
{
    QString ret(SystemComponent::info());

    ret += typeInfoString(typeid(this))
        + "\tFocal spot size: " + QString::number(_focalSpotSize.width()) + " mm x "
        + QString::number(_focalSpotSize.height())
        + " mm\n"
          "\tFocal spot position: "
        + QString::number(_focalSpotPosition(0, 0)) + " mm, "
        + QString::number(_focalSpotPosition(1, 0)) + " mm, "
        + QString::number(_focalSpotPosition(2, 0)) + " mm\n"
        + "\tFlux modifier: "
        + QString::number(_fluxModifier) + "\n";

    ret += (this->type() == AbstractSource::Type) ? "}\n" : "";

    return ret;
}

/*!
 * Sets the spectrum model to \a model.
 *
 * This instance manages ownership of \a model as a std::shared_ptr.
 */
inline void AbstractSource::setSpectrumModel(AbstractXraySpectrumModel* model)
{
    _spectrumModel.ptr.reset(model);
}

/*!
 * Sets the spectrum model to \a model.
 */
inline void AbstractSource::setSpectrumModel(std::unique_ptr<AbstractXraySpectrumModel> model)
{
    _spectrumModel.ptr = std::move(model);
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void AbstractSource::read(const QJsonObject& json)
{
    SystemComponent::read(json);

    QJsonArray fsPos = json.value("focal spot position").toArray();
    Vector3x1 fsPosVec({ fsPos.at(0).toDouble(), fsPos.at(1).toDouble(), fsPos.at(2).toDouble() });

    QJsonObject fsSize = json.value("focal spot size").toObject();
    QSizeF fsQSize;
    fsQSize.setWidth(fsSize.value("width").toDouble());
    fsQSize.setHeight(fsSize.value("height").toDouble());

    QVariant specMod = json.value("spectrum model").toVariant();

    _focalSpotSize = fsQSize;
    _focalSpotPosition = fsPosVec;
    _spectrumModel.ptr->fromVariant(specMod);
}

/*!
 * Writes all member variables to the QJsonObject \a json. Also writes the component's type-id
 * and generic type-id.
 */
inline void AbstractSource::write(QJsonObject& json) const
{
    SystemComponent::write(json);

    QJsonArray fsPos;
    fsPos.append(_focalSpotPosition.get<0>());
    fsPos.append(_focalSpotPosition.get<1>());
    fsPos.append(_focalSpotPosition.get<2>());

    QJsonObject fsSize;
    fsSize.insert("width", _focalSpotSize.width());
    fsSize.insert("height", _focalSpotSize.height());

    QJsonValue specMod = QJsonValue::fromVariant(_spectrumModel.ptr->toVariant());

    json.insert("focal spot position", fsPos);
    json.insert("focal spot size", fsSize);
    json.insert("spectrum model", specMod);
}

/*!
 * Reads all member variables from the QJsonObject \a json.
 */
inline void AbstractSource::fromVariant(const QVariant& variant)
{
    SystemComponent::fromVariant(variant);

    QVariantMap varMap = variant.toMap();
    auto fsPos = varMap.value("focal spot position").toList();
    Vector3x1 fsPosVec({ fsPos.at(0).toDouble(), fsPos.at(1).toDouble(), fsPos.at(2).toDouble() });

    auto fsSize = varMap.value("focal spot size").toMap();
    QSizeF fsQSize;
    fsQSize.setWidth(fsSize.value("width").toDouble());
    fsQSize.setHeight(fsSize.value("height").toDouble());

    QVariant specMod = varMap.value("spectrum model");

    _focalSpotSize = fsQSize;
    _focalSpotPosition = fsPosVec;
    _spectrumModel.ptr.reset(JsonSerializer::parseDataModel(specMod));
}

/*!
 * Stores all member variables in a QVariant. Also includes the component's type-id
 * and generic type-id.
 */
inline QVariant AbstractSource::toVariant() const
{
    QVariantMap ret = SystemComponent::toVariant().toMap();

    QVariantList fsPos;
    fsPos.append(_focalSpotPosition.get<0>());
    fsPos.append(_focalSpotPosition.get<1>());
    fsPos.append(_focalSpotPosition.get<2>());

    QVariantMap fsSize;
    fsSize.insert("width", _focalSpotSize.width());
    fsSize.insert("height", _focalSpotSize.height());

    QVariant specMod = _spectrumModel.ptr->toVariant();

    ret.insert("focal spot position", fsPos);
    ret.insert("focal spot size", fsSize);
    ret.insert("spectrum model", specMod);

    return ret;
}

/*!
 * Returns the photon flux (i.e. photons per cm²) emitted by the source in a distance of one meter.
 *
 * This method returns the nominal photon flux (as returned by nominalPhotonFlux()) multiplied with
 * the flux modifier.
 *
 * So far, the emitted flux cannot be direction-dependent (planned for future releases). However,
 * direction-dependency can be introduced using a beam modifier (see AbstractBeamModifier).
 */
inline double AbstractSource::photonFlux() const { return _fluxModifier * nominalPhotonFlux(); }

/*!
 * Returns the flux modifier.
 *
 * The flux modifier is a multiplicative factor that modifies the total photon flux returned by
 * photonFlux().
 */
inline double AbstractSource::fluxModifier() const { return _fluxModifier; }

/*!
 * Sets the flux modifier to \a modifier.
 *
 * The flux modifier is a multiplicative factor considered when querying the photon flux of the
 * source component. This value can be used to describe global variation of the emitted photon flux.
 * However, local solutions in sub-classes of AbstractSource are recommended when trying to model
 * intensity variation. Best practice is to realize this within the (re-)implementation of
 * nominalPhotonFlux() by utilization of dedicated class members (e.g. tube emission current).
 *
 * \sa photonFlux().
 */
inline void AbstractSource::setFluxModifier(double modifier) { _fluxModifier = modifier; }

} // namespace CTL

#endif // ABSTRACTSOURCE_H
