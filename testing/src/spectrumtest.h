#ifndef SPECTRUMTEST_H
#define SPECTRUMTEST_H

#include <QObject>
#include <QtTest>

namespace CTL
{
class XrayTube;
class XraySpectrumTabulatedModel;
class IntervalDataSeries;
}

class SpectrumTest : public QObject
{
    Q_OBJECT
public:
    SpectrumTest() = default;

private Q_SLOTS:
    void initTestCase();
    void testXrayLaserSpectrum();
    void testSpectrumSampling();
    void cleanupTestCase();
    void testAttenuationFilter();

private:
    CTL::XrayTube* _tube;
    CTL::XraySpectrumTabulatedModel* _model;

    float calcIntensity(float energy) const;
    float calcAnalyticIntegral(float from, float to) const;
    bool verifySampledSpectrum(const CTL::IntervalDataSeries& sampledSpec) const;

    float _m;
    float _n;
};

#endif // SPECTRUMTEST_H
