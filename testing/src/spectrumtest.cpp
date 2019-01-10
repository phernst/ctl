#include "spectrumtest.h"
#include <QDebug>
#include <QTime>

#include "components/xraylaser.h"

void SpectrumTest::initTestCase()
{
    const int TABLE_SMPL = 40;
    const float ENRG_MIN = 0.0f;
    const float ENRG_MAX = 100.0f;

    qsrand(uint(QTime::currentTime().msecsSinceStartOfDay()));

    _m = float(qrand()) / RAND_MAX * 5.0f;
    _n = float(qrand()) / RAND_MAX * 5.0f;

    QMap<float,float> table;
    for(int i=0; i<TABLE_SMPL; ++i)
    {
        float randVal   = float(qrand()) / RAND_MAX ;
        float energy    = randVal * (ENRG_MAX - ENRG_MIN) + ENRG_MIN;
        float intensity = calcIntensity(energy);

        table.insert(energy, intensity);
    }

    // add entries for minimum and maximum energy to lookup table
    table.insert(ENRG_MIN, calcIntensity(ENRG_MIN));
    table.insert(ENRG_MAX, calcIntensity(ENRG_MAX));

    _model = new CTL::XraySpectrumTabulatedModel;
    _model->addLookupTable(1.0, table);
    _model->addLookupTable(2.0, table);

    _tube = new CTL::XrayTube();
    //_tube->setSpectrumModel(_model);

    qInfo() << _tube->spectrum(0.0f, 120.0f, 100).normalized().values();
    qInfo() << _tube->spectrum(0.0f, 120.0f, 100).normalized().samplePoints();
}

void SpectrumTest::testXrayLaserSpectrum()
{
    CTL::XrayLaser laser;
    qInfo().noquote() << laser.spectrum(10.0, 110.0, 10).toMap();
    laser.setPhotonEnergy(55.0);
    qInfo().noquote() << laser.spectrum(0.0, 100.0, 10).toMap();
}

void SpectrumTest::testSpectrumSampling()
{
    _model->setParameter(1.5f);
    QVERIFY2(verifySampledSpectrum(CTL::SampledDataSeries(0.0f, 100.0f, 10, *_model)), "spectrum 1 failed");
    QVERIFY2(verifySampledSpectrum(CTL::SampledDataSeries(10.0f, 15.0f, 30, *_model)), "spectrum 2 failed");
    QVERIFY2(verifySampledSpectrum(CTL::SampledDataSeries(50.0f, 90.0f, 50, *_model)), "spectrum 3 failed");
}

void SpectrumTest::cleanupTestCase()
{
    delete _tube;
}

float SpectrumTest::calcIntensity(float energy) const
{
    return _m * energy + _n;
}

float SpectrumTest::calcAnalyticIntegral(float from, float to) const
{
    double d_from = static_cast<double>(from);
    double d_to = static_cast<double>(to);
    return static_cast<float>(0.5 * double(_m) * (d_to * d_to - d_from * d_from)
                              + double(_n) * (d_to - d_from));
}

bool SpectrumTest::verifySampledSpectrum(const CTL::SampledDataSeries &sampledSpec) const
{
    static const float eps = 1.0e-5f;

    uint errors = 0;
    for(uint i=0; i<sampledSpec.nbSamples(); ++i)
    {
        float from = sampledSpec.samplePoint(i) - 0.5f * sampledSpec.spacing();
        float to   = from + sampledSpec.spacing();
        if( fabs(calcAnalyticIntegral(from, to) - sampledSpec.value(i)) > eps * calcAnalyticIntegral(from, to))
        {
            qInfo() << "deviation detected: [" << sampledSpec.samplePoint(i) << " keV] " <<  sampledSpec.value(i) <<
                       " (analytic: " << calcAnalyticIntegral(from, to) << ")";
            ++errors;
        }
    }

    return errors == 0u;
}
