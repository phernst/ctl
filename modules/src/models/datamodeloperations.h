#ifndef CTL_DATAMODELOPERATIONS_H
#define CTL_DATAMODELOPERATIONS_H

#include "abstractdatamodel.h"

namespace CTL {

class AbstractDataModelOperation : public AbstractDataModel
{
public:
    AbstractDataModelOperation(std::shared_ptr<AbstractDataModel> lhs,
                               std::shared_ptr<AbstractDataModel> rhs);

    QVariant parameter() const override;
    void setParameter(const QVariant& parameter) override;

    ~AbstractDataModelOperation() override = default;

protected:
    std::shared_ptr<AbstractDataModel> _lhs;
    std::shared_ptr<AbstractDataModel> _rhs;

    AbstractDataModelOperation() = default;
    AbstractDataModelOperation(const AbstractDataModelOperation&) = default;
    AbstractDataModelOperation(AbstractDataModelOperation&&) = default;
    AbstractDataModelOperation& operator=(const AbstractDataModelOperation&) = default;
    AbstractDataModelOperation& operator=(AbstractDataModelOperation&&) = default;
};

class DataModelAdd : public AbstractDataModelOperation
{
    CTL_TYPE_ID(1)

    // abstract interface
    public:virtual float valueAt(float position) const override;
    public:virtual AbstractDataModel* clone() const override;

public:
    using AbstractDataModelOperation::AbstractDataModelOperation;
};

class DataModelSub : public AbstractDataModelOperation
{
    CTL_TYPE_ID(2)

    // abstract interface
    public:virtual float valueAt(float position) const override;
    public:virtual AbstractDataModel* clone() const override;

public:
    using AbstractDataModelOperation::AbstractDataModelOperation;
};

class DataModelMul : public AbstractDataModelOperation
{
    CTL_TYPE_ID(3)

    // abstract interface
    public:virtual float valueAt(float position) const override;
    public:virtual AbstractDataModel* clone() const override;

public:
    using AbstractDataModelOperation::AbstractDataModelOperation;
};

class DataModelDiv : public AbstractDataModelOperation
{
    CTL_TYPE_ID(4)

    // abstract interface
    public:virtual float valueAt(float position) const override;
    public:virtual AbstractDataModel* clone() const override;

public:
    using AbstractDataModelOperation::AbstractDataModelOperation;
};

class DataModelCat : public AbstractDataModelOperation
{
    CTL_TYPE_ID(5)

    // abstract interface
    public:virtual float valueAt(float position) const override;
    public:virtual AbstractDataModel* clone() const override;

public:
    using AbstractDataModelOperation::AbstractDataModelOperation;
};

} // namespace CTL

#endif // CTL_DATAMODELOPERATIONS_H
