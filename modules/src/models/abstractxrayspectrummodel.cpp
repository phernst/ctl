#include "abstractxrayspectrummodel.h"

namespace CTL {

// ____________________________
// # AbstractXraySpectrumModel
// ----------------------------
void AbstractXraySpectrumModel::setParameter(const QVariant &parameter)
{
    if(parameter.canConvert(QMetaType::Float))
        _energy = parameter.toFloat();
    else
        _energy = parameter.toMap().value("energy").toFloat();
}

QVariant AbstractXraySpectrumModel::parameter() const
{
    return QVariantMap{std::make_pair(QString("energy"), _energy)};
}

} // namespace CTL
