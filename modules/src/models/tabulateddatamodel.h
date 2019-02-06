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
class TabulatedDataModel : public AbstractIntegrableDataModel
{
    CTL_TYPE_ID(30)

    // abstract interfaces
    public: float valueAt(float position) const override;
    public: float binIntegral(float position, float binWidth) const override;
    public: AbstractDataModel* clone() const override;

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
    QVariant toVariant() const override;
    void fromVariant(const QVariant& variant) override;
    void insertDataPoint(float key, float value);

private:
    QMap<float, float> _data;

    QVariantList dataAsVariantList() const;
    void setDataFromVariantList(const QVariantList& list);
};

} // namespace CTL

#endif // TABULATEDDATAMODEL_H
