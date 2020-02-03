#ifndef ABSTRACTXRAYSPECTRUMMODEL_H
#define ABSTRACTXRAYSPECTRUMMODEL_H

#include "abstractdatamodel.h"

namespace CTL {

class AbstractXraySpectrumModel : public AbstractIntegrableDataModel
{
public:
    void setParameter(const QVariant& parameter) override;
    QVariant parameter() const override;

protected:
    float _energy = 0.0f; //!< Control parameter of device setting (usually tube voltage).
};

} // namespace CTL

#endif // ABSTRACTXRAYSPECTRUMMODEL_H
