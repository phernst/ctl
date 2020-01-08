#include "errormetrics.h"

#include <cmath>
#include <numeric>
#include <stdexcept>
#include <QtMath>

namespace CTL {

namespace metric {
    const ::CTL::imgproc::L1Norm L1;
    const ::CTL::imgproc::RelativeL1Norm rL1;
    const ::CTL::imgproc::L2Norm L2;
    const ::CTL::imgproc::RelativeL2Norm rL2;
    const ::CTL::imgproc::RMSE RMSE;
    const ::CTL::imgproc::RelativeRMSE rRMSE;
    const ::CTL::imgproc::CorrelationError corrErr;
    const ::CTL::imgproc::CosineSimilarityError cosSimErr;
    const ::CTL::imgproc::GemanMcClure GMCPreuhs(0.25);
    const ::CTL::imgproc::RelativeGemanMcClure rGMCPreuhs(0.25);
} // namespace metric

namespace imgproc {

double L1Norm::operator()(const std::vector<float>& first, const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("L1Norm::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();
    double ret = 0.0;
    for(size_t el = 0; el < nbEl; ++el)
        ret += std::fabs(first[el] - second[el]);

    return ret;
}

double RelativeL1Norm::operator()(const std::vector<float>& first,
                                  const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("RelativeL1Norm::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();
    double ret = 0.0;
    double ref = 0.0;
    for(size_t el = 0; el < nbEl; ++el)
    {
        ret += std::fabs(first[el] - second[el]);
        ref += std::fabs(first[el]);
    }

    return ret / ref;
}

double L2Norm::operator()(const std::vector<float>& first, const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("L2Norm::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();
    double ret = 0.0;
    for(size_t el = 0; el < nbEl; ++el)
        ret += std::pow(first[el] - second[el], 2.0);

    return std::sqrt(ret);
}

double RelativeL2Norm::operator()(const std::vector<float>& first,
                                  const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("RelativeL2Norm::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();
    double ret = 0.0;
    double ref = 0.0;
    for(size_t el = 0; el < nbEl; ++el)
    {
        ret += std::pow(first[el] - second[el], 2.0);
        ref += std::pow(first[el], 2.0);
    }

    return std::sqrt(ret) / std::sqrt(ref);
}

double RMSE::operator()(const std::vector<float>& first, const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("RMSE::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();

    return L2Norm{}(first, second) / std::sqrt(nbEl);
}

double RelativeRMSE::operator()(const std::vector<float>& first,
                                const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("RelativeRMSE::operator(): Vectors must have the same length.");

    return RelativeL2Norm{}(first, second);
}

double CorrelationError::operator()(const std::vector<float>& first,
                                    const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("Correlation::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();
    double mean1 = std::accumulate(first.cbegin(), first.cend(), 0.0) / double(nbEl);
    double mean2 = std::accumulate(second.cbegin(), second.cend(), 0.0) / double(nbEl);

    double numer = 0.0;
    double denom1 = 0.0;
    double denom2 = 0.0;
    for(size_t el = 0; el < nbEl; ++el)
    {
        numer += (first[el] - mean1) * (second[el] - mean2);
        denom1 += std::pow(first[el] - mean1, 2.0);
        denom2 += std::pow(second[el] - mean2, 2.0);
    }
    auto resDenom = denom1 * denom2;
    if(qFuzzyIsNull(resDenom))
    {
        qWarning("undefined correlation");
        return 1.0;
    }

    return 1.0 - (numer / std::sqrt(resDenom));
}

double CosineSimilarityError::operator()(const std::vector<float>& first,
                                         const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("CosineSimilarity::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();

    double numer = 0.0;
    double denom1 = 0.0;
    double denom2 = 0.0;
    for(size_t el = 0; el < nbEl; ++el)
    {
        numer += first[el] * second[el];
        denom1 += std::pow(first[el], 2.0);
        denom2 += std::pow(second[el], 2.0);
    }
    auto resDenom = denom1 * denom2;
    if(qFuzzyIsNull(resDenom))
    {
        qWarning("undefined cosine similarity");
        return 1.0;
    }

    return 1.0 - (numer / std::sqrt(resDenom));
}

double GemanMcClure::operator()(const std::vector<float>& first,
                                const std::vector<float>& second) const
{
    if(first.size() != second.size())
        throw std::domain_error("GemanMcClure::operator(): Vectors must have the same length.");

    const auto nbEl = first.size();
    const auto invPar = 1.0 / _parameter;

    double ret = 0.0;
    double squaredDiff;
    for(size_t el = 0; el < nbEl; ++el)
    {
        squaredDiff = std::pow(first[el] - second[el], 2.0);
        ret += squaredDiff / (1.0 + invPar * squaredDiff);
    }

    return ret;
}

double GemanMcClure::parameter() const { return _parameter; }

double RelativeGemanMcClure::operator()(const std::vector<float>& first,
                                        const std::vector<float>& second) const
{
    return GemanMcClure{ _parameter }(first, second) / (_parameter * double(first.size()));
}

double RelativeGemanMcClure::parameter() const { return _parameter; }

} // namespace imgproc
} // namespace CTL
