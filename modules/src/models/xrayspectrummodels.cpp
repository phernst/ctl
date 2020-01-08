#include "xrayspectrummodels.h"
#include <cmath>
#include <QDebug>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(XraySpectrumTabulatedModel)
DECLARE_SERIALIZABLE_TYPE(XrayLaserSpectrumModel)
DECLARE_SERIALIZABLE_TYPE(FixedXraySpectrumModel)
DECLARE_SERIALIZABLE_TYPE(KramersLawSpectrumModel)
DECLARE_SERIALIZABLE_TYPE(HeuristicCubicSpectrumModel)
DECLARE_SERIALIZABLE_TYPE(TASMIPSpectrumModel)

// _____________________________
// # XraySpectrumTabulatedModel
// -----------------------------
float XraySpectrumTabulatedModel::valueAt(float position) const
{
    if(!hasTabulatedDataFor(_energy))
        throw std::domain_error("No tabulated data available for parameter value: "
                                + std::to_string(_energy));

    // check if exact lookup is available --> return corresponding data
    if(_lookupTables.contains(_energy))
        return _lookupTables.value(_energy).valueAt(position);

    // data needs to be interpolated for requested value of '_parameter'
    auto parPosition = _lookupTables.upperBound(_energy);
    // --> always gives next tabulated data, since no exact lookup available

    const auto& tableLower = (parPosition - 1).value(); // tabulated data with voltage < _parameter
    const auto& tableUpper = (parPosition).value(); // tabulated data with voltage > _parameter

    auto lowerValue = tableLower.valueAt(position);
    auto upperValue = tableUpper.valueAt(position);

    // linear interpolation weighting
    auto weightFactor
        = (parPosition.key() - _energy) / (parPosition.key() - (parPosition - 1).key());

    return lowerValue * weightFactor + upperValue * (1.0f - weightFactor);
}

float XraySpectrumTabulatedModel::binIntegral(float position, float binWidth) const
{
    //    qDebug() << "value at called with: " << _parameter << samplePoint << spacing;

    if(!hasTabulatedDataFor(_energy))
        throw std::domain_error("No tabulated data available for parameter value: "
                                + std::to_string(_energy));

    // check if exact lookup is available --> return corresponding data
    if(_lookupTables.contains(_energy))
        return _lookupTables.value(_energy).binIntegral(position, binWidth);

    // data needs to be interpolated for requested value of '_parameter'
    auto parPosition = _lookupTables.upperBound(_energy);
    // --> always gives next tabulated data, since no exact lookup available

    auto tableLower = (parPosition - 1).value(); // tabulated data with voltage < _parameter
    auto tableUpper = (parPosition).value(); // tabulated data with voltage > _parameter

    //    qDebug() << _parameter << ":" << parPosition.key() << parPosition.value();
    //    qDebug() << "call Integrate: [" << samplePoint-spacing/2.0 << "," <<
    //    samplePoint+spacing/2.0 << "] ";

    auto lowerIntegral = tableLower.binIntegral(position, binWidth);
    auto upperIntegral = tableUpper.binIntegral(position, binWidth);

    //    qDebug() << "lowerIntegral = " << lowerIntegral;
    //    qDebug() << "upperIntegral = " << upperIntegral;

    // linear interpolation weighting
    auto weightFactor
        = (parPosition.key() - _energy) / (parPosition.key() - (parPosition - 1).key());

    return lowerIntegral * weightFactor + upperIntegral * (1.0f - weightFactor);
}

AbstractDataModel *XraySpectrumTabulatedModel::clone() const
{
    return new XraySpectrumTabulatedModel(*this);
}

QVariant XraySpectrumTabulatedModel::parameter() const
{
    auto variant = AbstractXraySpectrumModel::parameter().toMap();
    QVariantList dataList;

    auto i = _lookupTables.constBegin();
    while (i != _lookupTables.constEnd())
    {
        QVariantMap map;
        map.insert("table voltage", i.key());
        map.insert("table data", i.value().parameter());

        dataList.append(map);
        ++i;
    }

    variant.insert("lookup tables", dataList);

    return variant;
}

void XraySpectrumTabulatedModel::setParameter(const QVariant& parameter)
{
    AbstractXraySpectrumModel::setParameter(parameter);

    if(parameter.toMap().contains("lookup tables"))
    {
        _lookupTables.clear();

        // populate lookup table
        auto lookupTableData = parameter.toMap().value("lookup tables").toList();
        foreach(const QVariant& var, lookupTableData)
        {
            // each "var" represents a lookup table for a certain tube voltage
            auto varAsMap = var.toMap();
            auto voltage = varAsMap.value("table voltage").toFloat();
            auto tableData = varAsMap.value("table data");

            TabulatedDataModel table;
            table.setParameter(tableData);

            addLookupTable(voltage, table);
        }
    }
}


void XraySpectrumTabulatedModel::setLookupTables(const QMap<float, TabulatedDataModel>& tables)
{
    _lookupTables = tables;
}

void XraySpectrumTabulatedModel::addLookupTable(float voltage, const TabulatedDataModel& table)
{
    _lookupTables.insert(voltage, table);
}

bool XraySpectrumTabulatedModel::hasTabulatedDataFor(float voltage) const
{
    if(_lookupTables.isEmpty())
        return false;

    return (voltage >= _lookupTables.firstKey() && voltage <= (_lookupTables.lastKey()));
}


// _____________________________
// # XrayLaserSpectrumModel
// -----------------------------
float XrayLaserSpectrumModel::valueAt(float position) const
{
    if(qFuzzyCompare(position,_energy))
        return 1.0f;
    else
        return 0.0f;
}


float XrayLaserSpectrumModel::binIntegral(float position, float binWidth) const
{
    if((_energy >= position - 0.5f*binWidth) && (_energy <= position + 0.5f*binWidth))
        return 1.0f;
    else
        return 0.0f;
}

AbstractDataModel *XrayLaserSpectrumModel::clone() const
{
    return new XrayLaserSpectrumModel(*this);
}


// _____________________________
// # FixedXraySpectrumModel
// -----------------------------
FixedXraySpectrumModel::FixedXraySpectrumModel(const TabulatedDataModel &table)
{
    addLookupTable(0.0f, table);
}

void FixedXraySpectrumModel::setParameter(const QVariant &parameter)
{
    if(parameter.canConvert(QMetaType::Float))
    {
        qWarning() << "FixedXraySpectrumModel::setParameter(): Setting energy parameter is not "
                      "supported in FixedXraySpectrumModel. This call is ignored!";
        return;
    }

    if(parameter.toMap().value("energy", 0.0f).toFloat() != 0.0f)
    {
        qWarning() << "FixedXraySpectrumModel::setParameter(): Setting energy parameter is not "
                      "supported in FixedXraySpectrumModel. The corresponding entry in the "
                      "parameters is ignored!";
    }

    if(parameter.toMap().contains("lookup tables"))
    {
        // populate lookup table
        auto lookupTableData = parameter.toMap().value("lookup tables").toList();
        if(lookupTableData.isEmpty())
            return;
        if(lookupTableData.size() > 1)
            qWarning() << "FixedXraySpectrumModel::setParameter(): Parameters contain more than "
                          "one lookup table. Ignoring all tables but the first!";

        // parse the (first) lookup table
        auto varAsMap = lookupTableData.first().toMap();
        auto tableData = varAsMap.value("table data");

        TabulatedDataModel table;
        table.setParameter(tableData);

        setLookupTable(table);
    }
}

void FixedXraySpectrumModel::setLookupTable(const TabulatedDataModel& table)
{
    QMap<float, TabulatedDataModel> tmp;
    tmp.insert(0.0f, table);
    setLookupTables(tmp);
}

// _____________________________
// # KramersLawSpectrumModel
// -----------------------------
float KramersLawSpectrumModel::valueAt(float position) const
{
    return (position<_energy) ? (_energy / position - 1.0f) : 0.0f;
}

float KramersLawSpectrumModel::binIntegral(float position, float binWidth) const
{
    static constexpr float LOW_END = 0.1f;

    float bot = position - 0.5f*binWidth;
    float top = position + 0.5f*binWidth;

    if((top < LOW_END) || (bot > _energy))
        return 0.0f;

    if(bot < LOW_END)
        bot = LOW_END;
    if(top > _energy)
        top = _energy;

    return _energy * std::log(top / bot) - (top - bot);
}

AbstractDataModel *KramersLawSpectrumModel::clone() const
{
    return new KramersLawSpectrumModel(*this);
}

float HeuristicCubicSpectrumModel::valueAt(float position) const
{
    return (position < _energy) ? _energy * std::pow(_energy - position, 2.0f) - std::pow(_energy - position, 3.0f)
                                : 0.0f;
}

float HeuristicCubicSpectrumModel::binIntegral(float position, float binWidth) const
{
    auto antiderivative = [this](float E)
    {
        return (-1.0f / 3.0f) * _energy * std::pow(_energy - E, 3.0f) +
               (1.0f / 4.0f) * std::pow(_energy - E, 4.0f);
    };

    const float bot = position - 0.5f*binWidth;
    float top = position + 0.5f*binWidth;

    if((top < 0.0f) || (bot > _energy))
        return 0.0f;

    if(top > _energy)
        top = _energy;

    return antiderivative(top) - antiderivative(bot);
}

AbstractDataModel* HeuristicCubicSpectrumModel::clone() const
{
    return new HeuristicCubicSpectrumModel(*this);
}

float TASMIPSpectrumModel::valueAt(float position) const
{
    _tasmipData->setParameter(_energy);
    return _tasmipData->valueAt(position);
}

float TASMIPSpectrumModel::binIntegral(float position, float binWidth) const
{
    _tasmipData->setParameter(_energy);
    return _tasmipData->binIntegral(position, binWidth);
}

AbstractDataModel* TASMIPSpectrumModel::clone() const
{
    return new TASMIPSpectrumModel(*this);
}

void TASMIPSpectrumModel::setParameter(const QVariant& parameter)
{
    if(parameter.toFloat() > 140.0f)
        qWarning() << "Trying to set energy parameter to " + QString::number(parameter.toFloat()) +
                      ". TASMIP data is only available up to 140 kV.";

    AbstractXraySpectrumModel::setParameter(parameter);
}

TASMIPSpectrumModel::TASMIPSpectrumModel()
    : _tasmipData(makeDataModel<XraySpectrumTabulatedModel>())
{
    initializeModelData();
}

/*
 * TASMIP coefficients taken from
 *
 * Author(s):     John Boone & J. Anthony Seibert
 *
 * Article Title: An accurate method for computer-generating tungsten anode...
 *
 * Journal:       Medial Physics   E-MPHYA-24-1661
 * Issue Date:    November 1997    Volume: 24  Issue No.: 11
 *
 * Source: http://ftp.aip.org/epaps/medical_phys/E-MPHYA-24-1661/genspec1.h
 *
 */
void TASMIPSpectrumModel::initializeModelData()
{
    static constexpr auto nbBins = 140u;

    static constexpr float coeff[nbBins][4]={
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 0.0 keV...0.5 keV
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 0.5 keV...1.5 keV
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 1.5 keV...2.5 keV
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 2.5 keV...3.5 keV
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 3.5 keV...4.5 keV
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // ...
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {+0.000000e+000,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {-2.470985e+000,+7.522494e-002,+1.601297e-004,+0.000000e+000},
    {-5.468520e+001,+2.825971e+000,-3.702585e-002,+1.685450e-004},
    {-1.149660e+002,+7.181666e+000,-1.041506e-001,+5.246942e-004},
    {-2.023117e+001,+7.523026e+000,-9.725916e-002,+5.262351e-004},
    {+3.440159e+002,+3.179575e+000,-3.306927e-002,+3.115530e-004},
    {+5.493292e+002,+1.507932e+001,-9.656648e-002,+5.142380e-004},
    {+1.032546e+003,+2.793458e+001,-1.517779e-001,+6.491026e-004},
    {+1.056836e+003,+5.305293e+001,+4.006209e-002,-7.164506e-004},
    {+1.098845e+003,+8.295003e+001,+3.061647e-001,-2.617126e-003},
    {+4.957978e+002,+1.470037e+002,-1.102818e-001,-1.354507e-003},
    {-1.437833e+002,+2.229100e+002,-6.206306e-001,+1.896847e-004},
    {-1.106664e+003,+2.770497e+002,-5.743618e-001,-1.066210e-003},
    {-2.281766e+003,+3.424422e+002,-5.793318e-001,-2.303580e-003},
    {-5.591722e+003,+4.724134e+002,-1.429958e+000,+5.049076e-004},
    {-9.340535e+003,+6.186368e+002,-2.407872e+000,+3.701711e-003},
    {-1.406504e+004,+7.760495e+002,-3.430400e+000,+6.646413e-003},
    {-1.920322e+004,+9.418671e+002,-4.544806e+000,+9.920156e-003},
    {-2.515954e+004,+1.130912e+003,-5.997636e+000,+1.441550e-002},
    {-3.151928e+004,+1.331120e+003,-7.556880e+000,+1.925802e-002},
    {-3.165938e+004,+1.293120e+003,-6.625241e+000,+1.593667e-002},
    {-3.197696e+004,+1.259429e+003,-5.721722e+000,+1.269609e-002},
    {-3.150203e+004,+1.213018e+003,-4.995401e+000,+1.068630e-002},
    {-3.404540e+004,+1.273283e+003,-5.440755e+000,+1.275048e-002},
    {-3.525747e+004,+1.267165e+003,-5.052590e+000,+1.140252e-002},
    {-3.659796e+004,+1.264495e+003,-4.698218e+000,+1.017435e-002},
    {-3.935522e+004,+1.325721e+003,-5.260133e+000,+1.251165e-002},
    {-4.239447e+004,+1.396684e+003,-5.961586e+000,+1.539180e-002},
    {-4.505477e+004,+1.445302e+003,-6.324550e+000,+1.657817e-002},
    {-4.807436e+004,+1.506528e+003,-6.841015e+000,+1.832282e-002},
    {-4.772176e+004,+1.455009e+003,-6.183720e+000,+1.609850e-002},
    {-4.687265e+004,+1.383587e+003,-5.296423e+000,+1.305337e-002},
    {-4.534002e+004,+1.304458e+003,-4.458635e+000,+1.029127e-002},
    {-4.729671e+004,+1.337299e+003,-4.768113e+000,+1.129840e-002},
    {-4.592165e+004,+1.239852e+003,-3.651701e+000,+7.505117e-003},
    {-4.417617e+004,+1.131552e+003,-2.422704e+000,+3.340713e-003},
    {-4.975325e+004,+1.307914e+003,-4.490898e+000,+1.093279e-002},
    {-5.613191e+004,+1.511968e+003,-6.875300e+000,+1.962943e-002},
    {-5.524074e+004,+1.421870e+003,-5.669106e+000,+1.487642e-002},
    {-5.449938e+004,+1.337319e+003,-4.527925e+000,+1.035718e-002},
    {-5.884185e+004,+1.478833e+003,-6.293272e+000,+1.687622e-002},
    {-6.310984e+004,+1.616216e+003,-8.009326e+000,+2.321589e-002},
    {-5.995594e+004,+1.496680e+003,-6.906032e+000,+1.977848e-002},
    {-5.964100e+004,+1.456697e+003,-6.534316e+000,+1.853666e-002},
    {-6.132553e+004,+1.489142e+003,-6.956800e+000,+2.005068e-002},
    {-6.304895e+004,+1.522434e+003,-7.390895e+000,+2.161122e-002},
    {-5.994340e+004,+1.380871e+003,-5.839743e+000,+1.619943e-002},
    {-5.610868e+004,+1.218272e+003,-4.092096e+000,+1.018410e-002},
    {-1.825729e+004,-1.382119e+002,+9.557819e+000,-2.140051e-002},
    {+2.220017e+004,-1.568661e+003,+2.389806e+001,-5.505689e-002},
    {+5.501707e+004,-2.721157e+003,+3.527805e+001,-8.047399e-002},
    {+8.922944e+004,-3.915854e+003,+4.704985e+001,-1.070557e-001},
    {+2.104991e+004,-1.557364e+003,+2.321886e+001,-5.134972e-002},
    {-5.076517e+004,+9.032211e+002,-1.579828e+000,+7.306299e-003},
    {-6.030789e+004,+1.202068e+003,-4.552311e+000,+1.419530e-002},
    {-6.984994e+004,+1.499854e+003,-7.513087e+000,+2.103801e-002},
    {-7.108636e+004,+1.507313e+003,-7.472137e+000,+2.024801e-002},
    {-7.327537e+004,+1.540893e+003,-7.689933e+000,+2.028554e-002},
    {-3.161176e+004,+1.297773e+002,+6.392479e+000,-1.693738e-002},
    {+1.036295e+004,-1.288012e+003,+2.051981e+001,-5.423905e-002},
    {-4.132485e+004,+4.420904e+002,+2.448595e+000,+2.202247e-005},
    {-9.983141e+004,+2.351143e+003,-1.722188e+001,+5.896824e-002},
    {-8.345827e+004,+1.820261e+003,-1.140761e+001,+3.474510e-002},
    {-6.038053e+004,+1.099142e+003,-3.836391e+000,+5.215208e-003},
    {-7.332230e+004,+1.472738e+003,-7.481134e+000,+1.644730e-002},
    {-8.866886e+004,+1.911744e+003,-1.172736e+001,+2.948703e-002},
    {-8.906282e+004,+1.903695e+003,-1.166640e+001,+2.953372e-002},
    {-9.122084e+004,+1.949906e+003,-1.212404e+001,+3.119028e-002},
    {-9.195919e+004,+1.956641e+003,-1.222022e+001,+3.155684e-002},
    {-9.393503e+004,+1.997570e+003,-1.264453e+001,+3.294245e-002},
    {-9.460591e+004,+1.985575e+003,-1.240631e+001,+3.188458e-002},
    {-9.465909e+004,+1.947305e+003,-1.191912e+001,+3.005542e-002},
    {-1.054958e+005,+2.287738e+003,-1.546565e+001,+4.192772e-002},
    {-1.128820e+005,+2.523280e+003,-1.806383e+001,+5.099440e-002},
    {-5.652375e+004,+8.460812e+002,-1.890296e+000,+0.000000e+000},
    {-6.253113e+004,+9.546213e+002,-2.421458e+000,+0.000000e+000},
    {-6.063249e+004,+9.093265e+002,-2.222830e+000,+0.000000e+000},
    {-5.839087e+004,+8.581494e+002,-1.999379e+000,+0.000000e+000},
    {-6.177439e+004,+9.096954e+002,-2.219623e+000,+0.000000e+000},
    {-6.551339e+004,+9.674375e+002,-2.466158e+000,+0.000000e+000},
    {-6.482105e+004,+9.463755e+002,-2.384063e+000,+0.000000e+000},
    {-6.396586e+004,+9.225355e+002,-2.290526e+000,+0.000000e+000},
    {-5.976377e+004,+8.384694e+002,-1.918134e+000,+0.000000e+000},
    {-5.483239e+004,+7.418415e+002,-1.492676e+000,+0.000000e+000},
    {-5.545914e+004,+7.392220e+002,-1.466754e+000,+0.000000e+000},
    {-5.191874e+004,+6.677125e+002,-1.159438e+000,+0.000000e+000},
    {-5.337262e+004,+6.864440e+002,-1.248563e+000,+0.000000e+000},
    {-5.499713e+004,+7.080823e+002,-1.349865e+000,+0.000000e+000},
    {-6.109855e+004,+8.103042e+002,-1.805236e+000,+0.000000e+000},
    {-6.780313e+004,+9.224389e+002,-2.301017e+000,+0.000000e+000},
    {-6.463570e+004,+8.536160e+002,-1.980542e+000,+0.000000e+000},
    {-6.142322e+004,+7.841977e+002,-1.658250e+000,+0.000000e+000},
    {-6.542573e+004,+8.551263e+002,-1.999140e+000,+0.000000e+000},
    {-6.850218e+004,+9.104404e+002,-2.275249e+000,+0.000000e+000},
    {-6.775178e+004,+8.733046e+002,-2.050653e+000,+0.000000e+000},
    {-5.670986e+004,+6.717305e+002,-1.174642e+000,+0.000000e+000},
    {-6.431161e+004,+7.982173e+002,-1.730212e+000,+0.000000e+000},
    {-7.284777e+004,+9.397040e+002,-2.345359e+000,+0.000000e+000},
    {-7.296366e+004,+9.370416e+002,-2.349089e+000,+0.000000e+000},
    {-7.251969e+004,+9.256901e+002,-2.318580e+000,+0.000000e+000},
    {-7.373791e+004,+9.387560e+002,-2.371741e+000,+0.000000e+000},
    {-7.522138e+004,+9.557057e+002,-2.440560e+000,+0.000000e+000},
    {-6.645010e+004,+8.129935e+002,-1.892077e+000,+0.000000e+000},
    {-5.391723e+004,+6.111141e+002,-1.110798e+000,+0.000000e+000},
    {-6.950106e+004,+8.381854e+002,-1.943843e+000,+0.000000e+000},
    {-7.656837e+004,+9.340291e+002,-2.272803e+000,+0.000000e+000},
    {-7.169818e+004,+8.562692e+002,-1.994058e+000,+0.000000e+000},
    {-6.307650e+004,+7.199495e+002,-1.490337e+000,+0.000000e+000},
    {-6.896102e+004,+8.014658e+002,-1.785938e+000,+0.000000e+000},
    {-7.948799e+004,+9.545463e+002,-2.356450e+000,+0.000000e+000},
    {-8.038940e+004,+9.603943e+002,-2.368062e+000,+0.000000e+000},
    {-8.186549e+004,+9.744751e+002,-2.411129e+000,+0.000000e+000},
    {-8.127234e+004,+9.784392e+002,-2.501457e+000,+0.000000e+000},
    {-6.447853e+004,+7.327550e+002,-1.638994e+000,+0.000000e+000},
    {-3.806982e+004,+3.131658e+002,+0.000000e+000,+0.000000e+000},
    {-3.797812e+004,+3.101094e+002,+0.000000e+000,+0.000000e+000},
    {-4.023389e+004,+3.255209e+002,+0.000000e+000,+0.000000e+000},
    {-4.280943e+004,+3.432826e+002,+0.000000e+000,+0.000000e+000},
    {-4.114666e+004,+3.272756e+002,+0.000000e+000,+0.000000e+000},
    {-3.925966e+004,+3.096545e+002,+0.000000e+000,+0.000000e+000},
    {+3.191650e+002,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {-4.425804e+004,+3.425401e+002,+0.000000e+000,+0.000000e+000},
    {+8.115607e+001,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {-3.867988e+004,+2.969811e+002,+0.000000e+000,+0.000000e+000},
    {+1.306709e+003,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {+1.153422e+003,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {+9.817065e+002,+0.000000e+000,+0.000000e+000,+0.000000e+000},
    {+8.099662e+002,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // ...
    {+6.688839e+002,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 135.5 keV...136.5 keV
    {+5.277812e+002,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 136.5 keV...137.5 keV
    {+3.498336e+002,+0.000000e+000,+0.000000e+000,+0.000000e+000}, // 137.5 keV...138.5 keV
    {+1.718605e+002,+0.000000e+000,+0.000000e+000,+0.000000e+000}  // 138.5 keV...139.5 keV
    };

    static_assert(sizeof coeff == nbBins * 4 * sizeof(float),
                  "number of coefficients does not match.");

    float tubeVoltage = 0.0f;

    auto flux = [&tubeVoltage](uint energyBin) -> float
    {
        const auto binStart = energyBin - 0.5f;
        if(binStart > tubeVoltage)
            return 0.0f;

        const auto& coeffsOfBin = coeff[energyBin];
        const auto tubVoltagePow2 = tubeVoltage * tubeVoltage;

        const auto val = coeffsOfBin[0] +
                         coeffsOfBin[1] * tubeVoltage +
                         coeffsOfBin[2] * tubVoltagePow2 +
                         coeffsOfBin[3] * tubVoltagePow2 * tubeVoltage;

        return val > 0.0f ? val : 0.0f;
    };

    // prepare list of energies for each bin
    std::array<uint, nbBins> specBins;
    std::iota(specBins.begin(), specBins.end(), 0u); // 0 keV, 1 keV, 2 keV, ..., 139 keV

    // convert to floating point values
    QVector<float> specBinsF(nbBins);
    std::transform(specBins.cbegin(), specBins.cend(), specBinsF.begin(), [](uint binEnergy) {
        return float(binEnergy);
    });

    // compute spectrum for tube voltages in 1 kV steps: 0 kV, 1 kV, 2 kV, ..., 140 kV
    for(auto kV = 0u; kV <= nbBins; ++kV)
    {
        tubeVoltage = float(kV);
        QVector<float> specVals(nbBins);
        std::transform(specBins.cbegin(), specBins.cend(), specVals.begin(), flux);

        _tasmipData->addLookupTable(tubeVoltage, TabulatedDataModel(specBinsF, specVals));
    }
}

} // namespace CTL
