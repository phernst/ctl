#ifndef ERRORMETRICS_H
#define ERRORMETRICS_H

#include <vector>

typedef unsigned int uint;

namespace CTL {
namespace imgproc {

class AbstractErrorMetric
{
    public:virtual double operator()(const std::vector<float>& first,
                                     const std::vector<float>& second) const = 0;
public:
    virtual ~AbstractErrorMetric() = default;

protected:
    AbstractErrorMetric() = default;
    AbstractErrorMetric(const AbstractErrorMetric&) = default;
    AbstractErrorMetric(AbstractErrorMetric&&) = default;
    AbstractErrorMetric& operator=(const AbstractErrorMetric&) = default;
    AbstractErrorMetric& operator=(AbstractErrorMetric&&) = default;
};

class L1Norm : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class RelativeL1Norm : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class L2Norm : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class RelativeL2Norm : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class RMSE : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class RelativeRMSE : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class CorrelationError : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class CosineSimilarityError : public AbstractErrorMetric
{
public:
    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;
};

class GemanMcClure : public AbstractErrorMetric
{
public:
    GemanMcClure(double parameter) : _parameter(parameter) {}

    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;

    double parameter() const;

private:
    double _parameter;
};

class RelativeGemanMcClure : public AbstractErrorMetric
{
public:
    RelativeGemanMcClure(double parameter) : _parameter(parameter) {}

    double operator()(const std::vector<float>& first,
                      const std::vector<float>& second) const override;

    double parameter() const;

private:
    double _parameter;
};

} // namespace imgproc

namespace metric {
    extern const ::CTL::imgproc::L1Norm L1;
    extern const ::CTL::imgproc::RelativeL1Norm rL1;
    extern const ::CTL::imgproc::L2Norm L2;
    extern const ::CTL::imgproc::RelativeL2Norm rL2;
    extern const ::CTL::imgproc::RMSE RMSE;
    extern const ::CTL::imgproc::RelativeRMSE rRMSE;
    extern const ::CTL::imgproc::CorrelationError corrErr;
    extern const ::CTL::imgproc::CosineSimilarityError cosSimErr;
    extern const ::CTL::imgproc::GemanMcClure GMCPreuhs;
    extern const ::CTL::imgproc::RelativeGemanMcClure rGMCPreuhs;
} //namespace metric

} // namespace CTL

#endif // ERRORMETRICS_H
