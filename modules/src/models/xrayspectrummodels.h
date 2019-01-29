#ifndef XRAYSPECTRUMMODELS_H
#define XRAYSPECTRUMMODELS_H

#include "abstractdatamodel.h"
#include "tabulateddatamodel.h"

namespace CTL {

class AbstractXraySpectrumModel : public AbstractDensityDataModel
{
public:
    void setParameter(QVariant energy) override;
    QVariant parameter() const override;

protected:
    float _energy = 0.0f;
};

class XraySpectrumTabulatedModel : public AbstractXraySpectrumModel
{
    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: QVariant toVariant() const override;
    public: void fromVariant(const QVariant& variant) override;

public:
    void addLookupTable(float voltage, const TabulatedDataModel& table);
    void setLookupTables(const QMap<float, TabulatedDataModel>& tables);

    bool hasTabulatedDataFor(float voltage) const;

private:
    QMap<float, TabulatedDataModel> _lookupTables;

};

class XrayLaserSpectrumModel : public AbstractXraySpectrumModel
{
    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: QVariant toVariant() const override;
    public: void fromVariant(const QVariant& variant) override;
};

class FixedXraySpectrumModel : public XraySpectrumTabulatedModel
{
public:
    FixedXraySpectrumModel() = default;
    FixedXraySpectrumModel(const TabulatedDataModel& table);

    void setLookupTable(const TabulatedDataModel& table);
    void setParameter(QVariant energy) override;

private:
    using XraySpectrumTabulatedModel::addLookupTable;
    using XraySpectrumTabulatedModel::setLookupTables;
};

class KramersLawSpectrumModel : public AbstractXraySpectrumModel
{
    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: QVariant toVariant() const override;
    public: void fromVariant(const QVariant& variant) override;
};

} // namespace CTL

#endif // XRAYSPECTRUMMODELS_H
