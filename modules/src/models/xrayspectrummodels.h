#ifndef XRAYSPECTRUMMODELS_H
#define XRAYSPECTRUMMODELS_H

#include "abstractxrayspectrummodel.h"
#include "tabulateddatamodel.h"

namespace CTL {

class XraySpectrumTabulatedModel : public AbstractXraySpectrumModel
{
    CTL_TYPE_ID(35)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;

public:
    void addLookupTable(float voltage, const TabulatedDataModel& table);
    void setLookupTables(const QMap<float, TabulatedDataModel>& tables);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

    bool hasTabulatedDataFor(float voltage) const;

protected:
    QMap<float, TabulatedDataModel> _lookupTables;

};

class XrayLaserSpectrumModel : public AbstractXraySpectrumModel
{
    CTL_TYPE_ID(40)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;
};

class FixedXraySpectrumModel : public XraySpectrumTabulatedModel
{
    CTL_TYPE_ID(36)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;

public:
    FixedXraySpectrumModel() = default;
    FixedXraySpectrumModel(const TabulatedDataModel& table);

    void setParameter(const QVariant& parameter) override;

    void setLookupTable(const TabulatedDataModel& table);

private:
    using XraySpectrumTabulatedModel::addLookupTable;
    using XraySpectrumTabulatedModel::setLookupTables;
};

class KramersLawSpectrumModel : public AbstractXraySpectrumModel
{
    CTL_TYPE_ID(41)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;
};

class HeuristicCubicSpectrumModel : public AbstractXraySpectrumModel
{
    CTL_TYPE_ID(42)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;
};

class TASMIPSpectrumModel : public AbstractXraySpectrumModel
{
    CTL_TYPE_ID(43)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;

    void setParameter(const QVariant& parameter) override;

public:
    TASMIPSpectrumModel();

private:
    DataModelPtr<XraySpectrumTabulatedModel> _tasmipData;

    void initializeModelData();
};

} // namespace CTL

#endif // XRAYSPECTRUMMODELS_H
