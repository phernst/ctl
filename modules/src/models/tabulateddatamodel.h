#ifndef TABULATEDDATAMODEL_H
#define TABULATEDDATAMODEL_H

#include "abstractdatamodel.h"
#include <QMap>

namespace CTL {

/*!
 * \class TabulatedDataModel
 * \brief The TabulatedDataModel class is a data model that handles values in a lookup table.
 *
 * Sub-classes must implement the method to sample a value at a given position (valueAt()).
 *
 * Parameters can be set by passing a QVariant that contains all necessary information.
 * Re-implement the setParameter() method to parse the QVariant into your required format within
 * sub-classes of TabulatedDataModel.
 */
class TabulatedDataModel : public AbstractDensityDataModel
{
    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: QVariant toVariant() const override;
    public: void fromVariant(const QVariant& variant) override;

public:
    TabulatedDataModel() = default;
    TabulatedDataModel(QMap<float, float> table);
    TabulatedDataModel(const QVector<float>& keys, const QVector<float>& values);

    // getter methods
    const QMap<float, float>& lookupTable() const;

    // setter methods
    void setData(QMap<float, float> table);
    void setData(const QVector<float>& keys, const QVector<float>& values);

    // other methods
    void insertDataPoint(float key, float value);

private:
    QMap<float, float> _data;
};

} // namespace CTL

#endif // TABULATEDDATAMODEL_H
