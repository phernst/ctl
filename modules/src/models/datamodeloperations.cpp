#include "datamodeloperations.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(DataModelAdd)
DECLARE_SERIALIZABLE_TYPE(DataModelSub)
DECLARE_SERIALIZABLE_TYPE(DataModelMul)
DECLARE_SERIALIZABLE_TYPE(DataModelDiv)

CTL::AbstractDataModelOperation::AbstractDataModelOperation(std::shared_ptr<AbstractDataModel> lhs,
                                                            std::shared_ptr<AbstractDataModel> rhs)
    : _lhs(std::move(lhs))
    , _rhs(std::move(rhs))
{
    if(!_lhs || !_rhs)
        throw std::runtime_error("AbstractDataModelOperation: Unable to construct data model operation."
                                 "At least one of the operands is nullptr.");
}

QVariant AbstractDataModelOperation::parameter() const
{
    QVariantMap ret;
    ret.insert("LHS model", _lhs->toVariant());
    ret.insert("RHS model", _rhs->toVariant());

    return ret;
}

void AbstractDataModelOperation::setParameter(const QVariant& parameter)
{
    QVariantMap parMap = parameter.toMap();
    _lhs.reset(SerializationHelper::parseDataModel(parMap.value("LHS model")));
    _rhs.reset(SerializationHelper::parseDataModel(parMap.value("RHS model")));
}

float DataModelAdd::valueAt(float position) const
{
    return _lhs->valueAt(position) + _rhs->valueAt(position);
}

float DataModelSub::valueAt(float position) const
{
    return _lhs->valueAt(position) - _rhs->valueAt(position);
}

float DataModelMul::valueAt(float position) const
{
    return _lhs->valueAt(position) * _rhs->valueAt(position);
}

float DataModelDiv::valueAt(float position) const
{
    return _lhs->valueAt(position) / _rhs->valueAt(position);
}

AbstractDataModel* DataModelAdd::clone() const { return new DataModelAdd(*this); }

AbstractDataModel* DataModelSub::clone() const { return new DataModelSub(*this); }

AbstractDataModel* DataModelMul::clone() const { return new DataModelMul(*this); }

AbstractDataModel* DataModelDiv::clone() const { return new DataModelDiv(*this); }

} // namespace CTL
