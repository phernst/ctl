#ifndef CTL_ABSTRACTSOURCE_H
#define CTL_ABSTRACTSOURCE_H
#include "mat/matrix_types.h"
#include "models/intervaldataseries.h"
#include "models/abstractxrayspectrummodel.h"
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
 * The AbstractSource class has two abstract interface methods: energyRange() and
 * nominalPhotonFlux(). The energyRange() method must provide the energy boundaries within which
 * all radiation from the source is contained. This range will be used whenenver the source's
 * radiation spectrum is queried using spectrum() in order to determine the required interval for
 * a sampled representation of the spectrum.
 * The nominalPhotonFlux() method shall return the (unmodified) flux of photons emitted by the
 * source. This is used whithin the actual getter method photonFlux(), which returns the nominal
 * flux multiplied by an (optional, defaults to 1.0) modifier (see fluxModifier() and
 * setFluxModifier()).
 *
 * Spectrum computation is based on an AbstractXraySpectrumModel, which can be set using the
 * corresponding setter method setSpectrumModel(). To query the current spectrum, use the spectrum()
 * method, passing the number of desired sampling points as an input.
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
 * from SerializationInterface::UserType, as these are reserved for user-defined types.
 *
 * To enable de-/serialization of objects of the new sub-class, reimplement the toVariant() and
 * fromVariant() methods. These should take care of all newly introduced information of the
 * sub-class. Additionally, call the macro #DECLARE_SERIALIZABLE_TYPE(YourNewClassName) within the
 * .cpp file of your new class (substitute "YourNewClassName" with the actual class name). Objects
 * of the new class can then be de-/serialized with any of the serializer classes (see also
 * AbstractSerializer).
 */

typedef Range<float> EnergyRange;

class AbstractSource : public SystemComponent
{
    CTL_TYPE_ID(300)
    DECLARE_ELEMENTAL_TYPE

public:
    static const uint DEFAULT_SPECTRUM_RESOLUTION_HINT = 10;

    // abstract interface
    public:virtual EnergyRange nominalEnergyRange() const = 0;
    protected:virtual double nominalPhotonFlux() const = 0;

public:
    // virtual methods  
    virtual IntervalDataSeries spectrum(uint nbSamples) const;
    virtual uint spectrumDiscretizationHint() const;
    virtual void setSpectrumModel(AbstractXraySpectrumModel* model);
    QString info() const override;
    void fromVariant(const QVariant& variant) override; // de-serialization
    QVariant toVariant() const override; // serialization

    // getter methods
    EnergyRange energyRange() const;
    float meanEnergy() const;
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
    void setEnergyRangeRestriction(const EnergyRange& window);

    // other methods
    IntervalDataSeries spectrum(EnergyRange range, uint nbSamples) const;
    bool hasSpectrumModel() const;
    void setSpectrumModel(std::unique_ptr<AbstractXraySpectrumModel> model);

    ~AbstractSource() override = default;

protected:
    AbstractSource() = default;
    AbstractSource(const QString& name);
    AbstractSource(const QSizeF& focalSpotSize, const QString& name);
    AbstractSource(const QSizeF& focalSpotSize,
                   const Vector3x1& focalSpotPosition,
                   const QString& name);
    AbstractSource(const QSizeF& focalSpotSize,
                   const Vector3x1& focalSpotPosition,
                   AbstractXraySpectrumModel* spectumModel,
                   const QString& name);

    AbstractSource(const AbstractSource&) = default;
    AbstractSource(AbstractSource&&) = default;
    AbstractSource& operator=(const AbstractSource&) = default;
    AbstractSource& operator=(AbstractSource&&) = default;

    QSizeF _focalSpotSize = QSizeF(0.0, 0.0); //!< Size of the focal spot (in mm).
    Vector3x1 _focalSpotPosition = Vector3x1(0.0); //!< Position of the focal spot (relative to source center).
    double _fluxModifier = 1.0; //!< Global (multiplicative) modifier for the photon flux.

    DataModelPtr<AbstractXraySpectrumModel> _spectrumModel; //!< Data model for the emitted radiation spectrum.
    EnergyRange _restrictedEnergyWindow = { 0.0f, 0.0f }; //!< Windowed energy range.
    bool _hasRestrictedEnergyWindow = false;
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
 * \fn EnergyRange AbstractSource::energyRange() const
 *
 * Abstract interface method. Returns the energy range that contains all radiation from the source.
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
    , _spectrumModel(spectumModel)
{
}


/*!
 * Returns the emitted radiation spectrum sampled with \a nbSamples bins covering the energy
 * range of [energyRange().from, energyRange().to] keV. The method spectrumDiscretizationHint() can
 * provide a hint for a reasonable number of samples.
 *
 * Each energy bin in the returned data series is defined to represent the integral over the
 * contribution to the total intensity of all energies within that particular bin. The individual
 * contributions are extracted from the AbstractXraySpectrumModel set to the component.
 *
 * The returned spectrum contains relative intensities, i.e. the sum over all bins equals to one.
 *
 * Throws std::runtime_error if no spectrum model is available.
 *
 * \sa energyRange(), spectrumModel(), setSpectrumModel().
 */
inline IntervalDataSeries AbstractSource::spectrum(uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");

    const auto eRange = energyRange();
    auto spec = IntervalDataSeries::sampledFromModel(*_spectrumModel,
                                                     eRange.start(), eRange.end(), nbSamples);

    spec.normalizeByIntegral();

    return spec;
}


inline IntervalDataSeries AbstractSource::spectrum(EnergyRange range, uint nbSamples) const
{
    if(!hasSpectrumModel())
        throw std::runtime_error("No spectrum model set.");

    auto spec = IntervalDataSeries::sampledFromModel(*_spectrumModel,
                                                     range.start(), range.end(), nbSamples);
    if(_hasRestrictedEnergyWindow)
        spec.clampToRange({ _restrictedEnergyWindow.start(), _restrictedEnergyWindow.end() });

    spec.normalizeByIntegral();

    return spec;
}

/*!
 * Returns a hint for a reasonable number of sampling points when querying a spectrum of the
 * component. By default, this method returns 10. Re-implement this method in sub-classes to return
 * meaningful hints for your particular source component type.
 */
inline uint AbstractSource::spectrumDiscretizationHint() const
{
    return DEFAULT_SPECTRUM_RESOLUTION_HINT;
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
inline const AbstractXraySpectrumModel* AbstractSource::spectrumModel() const
{
    return _spectrumModel.get();
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

inline void AbstractSource::setEnergyRangeRestriction(const EnergyRange& window)
{
    _restrictedEnergyWindow = window;
    _hasRestrictedEnergyWindow = true;
}

/*!
 * Returns true if a spectrum model is available in this instance.
 *
 * \sa setSpectrumModel().
 */
inline bool AbstractSource::hasSpectrumModel() const { return static_cast<bool>(_spectrumModel); }

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
        + "\tEnergy range: [" + QString::number(energyRange().start()) + "," +
            QString::number(energyRange().end()) + "] keV\n"
        + "\tNominal photon flux: " + QString::number(nominalPhotonFlux())
            + "photons / cm^2 @ 1m\n"
        + "\tFlux modifier: "
        + QString::number(_fluxModifier) + "\n";
        + "\tFocal spot size: " + QString::number(_focalSpotSize.width()) + " mm x "
        + QString::number(_focalSpotSize.height())
        + " mm\n"
          "\tFocal spot position: "
        + QString::number(_focalSpotPosition(0, 0)) + " mm, "
        + QString::number(_focalSpotPosition(1, 0)) + " mm, "
        + QString::number(_focalSpotPosition(2, 0)) + " mm\n";

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
    _spectrumModel.reset(model);
}

/*!
 * Sets the spectrum model to \a model.
 */
inline void AbstractSource::setSpectrumModel(std::unique_ptr<AbstractXraySpectrumModel> model)
{
    _spectrumModel = std::move(model);
}

// Use SerializationInterface::fromVariant() documentation.
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
    _spectrumModel.reset(static_cast<AbstractXraySpectrumModel*>(
                             SerializationHelper::parseDataModel(specMod)));
}

// Use SerializationInterface::toVariant() documentation.
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

    QVariant specMod = _spectrumModel ? _spectrumModel->toVariant() : QVariant();

    ret.insert("focal spot position", fsPos);
    ret.insert("focal spot size", fsSize);
    ret.insert("spectrum model", specMod);

    return ret;
}

inline EnergyRange AbstractSource::energyRange() const
{
    const auto nomRange = nominalEnergyRange();
    if(!_hasRestrictedEnergyWindow)
        return nomRange;

    return { std::max(nomRange.start(), _restrictedEnergyWindow.start()),
                std::min(nomRange.end(), _restrictedEnergyWindow.end()) };
}

/*!
 * Returns the mean energy (in keV) of the spectrum emitted by this instance. Spectrum sampling is
 * performed using the number of samples returned by spectrumDiscretizationHint(). Note that, in
 * general, this is only an approximative mean.
 */
inline float AbstractSource::meanEnergy() const
{
    return spectrum(spectrumDiscretizationHint()).centroid();
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

/*! \file */
///@{
/*!
 * \typedef CTL::EnergyRange
 *
 * \brief Alias name for CTL::Range<float>.
 *
 * Holds the borders (i.e. minimum to maximum) of the energy range.
 *
 * \relates CTL::AbstractSource
 */
///@}

#endif // CTL_ABSTRACTSOURCE_H
