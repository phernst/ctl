#ifndef XRAYSPECTRUMMODELS_H
#define XRAYSPECTRUMMODELS_H

#include "abstractspectralmodel.h"
#include "tabulatedmodeldata.h"

namespace CTL {

class AbstractXraySpectrumModel : public AbstractParameterizedSpectralModel<float>
{
public:
    void setParameter(const float& voltage) override;
    const float& parameter() const override;

protected:
    float _parameter;
};

class XraySpectrumTabulatedModel : public AbstractXraySpectrumModel
{
public:
    QVariant toVariant() const override;
    void fromVariant(const QVariant& variant) override;

    void addLookupTable(float voltage, const TabulatedModelData& table);
    void setLookupTables(const QMap<float, TabulatedModelData>& tables);

    bool hasTabulatedDataFor(float voltage) const;

protected:
    float valueFromModel(float samplePoint, float spacing) const override;

private:
    QMap<float, TabulatedModelData> _lookupTables;
};

class XrayLaserSpectrumModel : public AbstractXraySpectrumModel
{
public:
    QVariant toVariant() const override;
    void fromVariant(const QVariant&) override;

protected:
    float valueFromModel(float samplePoint, float spacing) const override;
};

class GenericSpectrumModel : public AbstractSpectralModel
{
public:
    GenericSpectrumModel() = default;
    GenericSpectrumModel(const TabulatedModelData& table);

    QVariant toVariant() const override;
    void fromVariant(const QVariant& variant) override;

    void setLookupTable(TabulatedModelData table);

protected:
    float valueFromModel(float samplePoint, float spacing) const override;

private:
    TabulatedModelData _lookupTable;
};

class KramersLawSpectrumModel : public AbstractXraySpectrumModel
{
public:
    QVariant toVariant() const override;
    void fromVariant(const QVariant& variant) override;

protected:
    float valueFromModel(float samplePoint, float spacing) const override;
};

} // namespace CTL

#endif // XRAYSPECTRUMMODELS_H
