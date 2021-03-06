#include "spectrumtest.h"
#include <QDebug>
#include <QTime>

#include "components/attenuationfilter.h"
#include "components/xraylaser.h"
#include "components/xraytube.h"
#include "io/ctldatabase.h"
#include "models/xrayspectrummodels.h"

void SpectrumTest::initTestCase()
{
    const int TABLE_SMPL = 40;
    const float ENRG_MIN = 0.0f;
    const float ENRG_MAX = 100.0f;

    qsrand(uint(QTime::currentTime().msecsSinceStartOfDay()));

    _m = static_cast<float>(qrand()) / static_cast<float>(RAND_MAX) * 5.0f;
    _n = static_cast<float>(qrand()) / static_cast<float>(RAND_MAX) * 5.0f;

    QMap<float,float> table;
    for(int i=0; i<TABLE_SMPL; ++i)
    {
        float randVal   = static_cast<float>(qrand()) / static_cast<float>(RAND_MAX);
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

    qInfo() << _tube->spectrum(100).normalizedByIntegral().values();
    qInfo() << _tube->spectrum(100).normalizedByIntegral().samplingPoints();
}

void SpectrumTest::cleanupTestCase()
{
    delete _tube;
    delete _model;
}

void SpectrumTest::testAttenuationFilter()
{
    constexpr auto laserEnergy = 42.0;

    std::unique_ptr<CTL::AbstractSource> source;
    std::unique_ptr<CTL::AbstractBeamModifier> filter(new CTL::AttenuationFilter(CTL::database::Element::Al, 4.2f));
    CTL::IntervalDataSeries spectrum;
    double flux;

    source.reset(new CTL::XrayLaser(laserEnergy, 1.0));
    flux = filter->modifiedFlux(source->photonFlux(), source->spectrum(1));
    spectrum = filter->modifiedSpectrum(source->spectrum(1));
    QCOMPARE(spectrum.value(0), 1.0f);
    QVERIFY(flux < source->photonFlux());

    source.reset(new CTL::XrayTube(100.0, 0.1));
    flux = filter->modifiedFlux(source->photonFlux(), source->spectrum(10));
    spectrum = filter->modifiedSpectrum(source->spectrum(10));
    QVERIFY(flux < source->photonFlux());
    QVERIFY(spectrum.centroid() > source->spectrum(10).centroid());

}

void SpectrumTest::testXrayLaserSpectrum()
{
    CTL::XrayLaser laser;
    auto spectrum = laser.spectrum(2);
    qInfo().noquote() << spectrum.data();
    QCOMPARE(spectrum.value(0), 0.5f);
    QCOMPARE(spectrum.value(1), 0.5f);
    QCOMPARE(spectrum.samplingPoint(0), 100.0f);
    laser.setPhotonEnergy(55.0);
    spectrum = laser.spectrum(10);
    qInfo().noquote() << spectrum.data();
    QCOMPARE(spectrum.value(4), 0.1f);
    QCOMPARE(spectrum.samplingPoint(4), 55.0f);

}

void SpectrumTest::testSpectrumSampling()
{
    _model->setParameter(1.5f);
    QVERIFY2(verifySampledSpectrum(CTL::IntervalDataSeries::sampledFromModel(*_model, 0.0f, 100.0f, 10)), "spectrum 1 failed");
    QVERIFY2(verifySampledSpectrum(CTL::IntervalDataSeries::sampledFromModel(*_model, 10.0f, 15.0f, 30)), "spectrum 2 failed");
    QVERIFY2(verifySampledSpectrum(CTL::IntervalDataSeries::sampledFromModel(*_model, 50.0f, 90.0f, 50)), "spectrum 3 failed");
}

float SpectrumTest::calcIntensity(float energy) const
{
    return _m * energy + _n;
}

float SpectrumTest::calcAnalyticIntegral(float from, float to) const
{
    auto d_from = static_cast<double>(from);
    auto d_to = static_cast<double>(to);
    return static_cast<float>(0.5 * double(_m) * (d_to * d_to - d_from * d_from)
                              + double(_n) * (d_to - d_from));
}

bool SpectrumTest::verifySampledSpectrum(const CTL::IntervalDataSeries &sampledSpec) const
{
    static constexpr float eps = 1.0e-3f;

    uint errors = 0;
    for(uint i=0; i<sampledSpec.nbSamples(); ++i)
    {
        float from = sampledSpec.samplingPoint(i) - 0.5f * sampledSpec.binWidth();
        float to   = from + sampledSpec.binWidth();
        if( std::fabs(calcAnalyticIntegral(from, to) - sampledSpec.value(i)) > eps * calcAnalyticIntegral(from, to))
        {
            qInfo() << "deviation detected: [" << sampledSpec.samplingPoint(i) << " keV] " <<  sampledSpec.value(i) <<
                       "(analytic: " << calcAnalyticIntegral(from, to) << ")";
            ++errors;
        }
    }

    return errors == 0u;
}
