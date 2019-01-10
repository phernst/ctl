#ifndef TABULATEDMODELDATA_H
#define TABULATEDMODELDATA_H

#include <QMap>

namespace CTL {

class TabulatedModelData
{
public:
    TabulatedModelData() = default;
    TabulatedModelData(const QMap<float, float>& table);

    float trapezoidIntegral(float from, float to) const;
    float interpLin(float pos) const;

    // QVariant toQVariant() const; //TBD
    void setData(QMap<float, float> table);

private:
    QMap<float, float> _data;
};

} // namespace CTL

#endif // TABULATEDMODELDATA_H
