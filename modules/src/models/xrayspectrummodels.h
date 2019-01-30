#ifndef XRAYSPECTRUMMODELS_H
#define XRAYSPECTRUMMODELS_H

#include "abstractdatamodel.h"
#include "tabulateddatamodel.h"

namespace CTL {

class AbstractXraySpectrumModel : public AbstractIntegrableDataModel
{
public:
    void setParameter(const QVariant& parameter) override;
    QVariant parameter() const override;

protected:
    float _energy = 0.0f;
};

class XraySpectrumTabulatedModel : public AbstractXraySpectrumModel
{
    ADD_TO_MODEL_ENUM(35)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;

public:
    void addLookupTable(float voltage, const TabulatedDataModel& table);
    void setLookupTables(const QMap<float, TabulatedDataModel>& tables);

    QVariant toVariant() const override;
    void fromVariant(const QVariant& variant) override;

    bool hasTabulatedDataFor(float voltage) const;

private:
    QMap<float, TabulatedDataModel> _lookupTables;

};

class XrayLaserSpectrumModel : public AbstractXraySpectrumModel
{
    ADD_TO_MODEL_ENUM(40)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;
};

class FixedXraySpectrumModel : public XraySpectrumTabulatedModel
{
    ADD_TO_MODEL_ENUM(36)

public:
    FixedXraySpectrumModel() = default;
    FixedXraySpectrumModel(const TabulatedDataModel& table);

    void setLookupTable(const TabulatedDataModel& table);

private:
    using XraySpectrumTabulatedModel::addLookupTable;
    using XraySpectrumTabulatedModel::setLookupTables;
};

class KramersLawSpectrumModel : public AbstractXraySpectrumModel
{
    ADD_TO_MODEL_ENUM(41)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;
};

} // namespace CTL

#endif // XRAYSPECTRUMMODELS_H
