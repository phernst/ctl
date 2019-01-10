#ifndef SPECTRUMTEST_H
#define SPECTRUMTEST_H

#include <QObject>
#include <QtTest>
#include "components/xraytube.h"

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

private:
    CTL::XrayTube* _tube;
    CTL::XraySpectrumTabulatedModel* _model;

    float calcIntensity(float energy) const;
    float calcAnalyticIntegral(float from, float to) const;
    bool verifySampledSpectrum(const CTL::SampledDataSeries& sampledSpec) const;

    float _m;
    float _n;
};

#endif // SPECTRUMTEST_H
