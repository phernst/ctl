#ifndef TABULATEDDATAMODEL_H
#define TABULATEDDATAMODEL_H

#include "abstractdatamodel.h"
#include <QMap>

namespace CTL {

class TabulatedDataModel : public AbstractDensityDataModel
{
    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: QVariant toVariant() const override;
    public: void fromVariant(const QVariant& variant) override;

public:
    TabulatedDataModel() = default;
    TabulatedDataModel(const QMap<float, float>& table);

    void setData(QMap<float, float> table);

private:
    QMap<float, float> _data;
};

} // namespace CTL

#endif // TABULATEDDATAMODEL_H
