#include "ctl.h"
#include "ctl_nlopt.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QElapsedTimer>

static std::unique_ptr<CTL::io::AbstractProjectionMatrixIO> projMatIO(const QString& fn);
static std::unique_ptr<CTL::io::AbstractProjectionDataIO> projDataIO(const QString& fn);
static std::unique_ptr<CTL::io::AbstractVolumeIO<float>> volumeIO(const QString& fn);

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationVersion("1.3");

    QCommandLineParser parser;
    parser.setApplicationDescription("Grangeat-based 2D/3D Registration with pre-computed derivative of the 3D Radon space.");
    parser.addVersionOption();
    parser.addHelpOption();

    parser.addOptions({
        { { "p", "proj-mat" }, "File with projection matrices [NRRD/DEN].", "path" },
        { { "i", "proj-imgs" }, "File with 2D projection images [NRRD/DEN].", "path" },
        { { "r", "radon3d" }, "File with pre-computed 3D Radon space [NRRD].", "path" },

        { { "d", "diff" }, "Performes a central difference filter on the 3D Radon space w.r.t. the distance dimension." },

        { { "g", "global" }, "Enable global optimization." },
        { { "n", "no-subsampling" }, "Disable subsampling (same as '-s 1.0')." },
        { { "s", "sub-sampling" }, "Fraction of the sampled subset of available values. Default: 0.1", "fraction" },
        { { "l", "lower-bound" }, "Lower bound of opimization [mm/deg]. Default: -30.0", "value" },
        { { "u", "upper-bound" }, "Upper bound of opimization [mm/deg]. Default: 30.0", "value" },
        { { "m", "gmc-metric" }, "Parameter of the GMC metric. Default: 50.0", "value" },

        { { "j", "device-number" }, "Use only a specific OpenCL device with index 'number'.", "number" },

        { { "o", "output" }, "Output directory.", "path" }
    });

    // Process the actual command line arguments given by the user
    parser.process(app);

    if(!(parser.isSet("p") && parser.isSet("i") && parser.isSet("r")))
    {
        qCritical("Projection matrices, projection images and 3D volume must be specified."
                  "See p, i and v options in help.");
        return -1;
    }
    if(parser.isSet("j"))
        CTL::OCL::OpenCLConfig::instance().setDevices( {
            CTL::OCL::OpenCLConfig::instance().devices().at(parser.value("j").toUInt()) } );

    const QString fnProjMat{ parser.value("p") };
    const QString fnProjImg{ parser.value("i") };
    const QString fnRadon{ parser.value("r") };

    // Init IOs
    auto ioProjMat = projMatIO(fnProjMat);
    auto ioProjImg = projDataIO(fnProjImg);
    auto ioVol = volumeIO(fnRadon); if(!ioVol) return -1;

    // Load all Pmats
    auto Ps = ioProjMat->readFullGeometry(fnProjMat, 1);

    // Perform some checks
    if(Ps.nbViews() < 1)
    {
        qCritical("No projection matrices.");
        return -1;
    }
    if(Ps.at(0).nbModules() != 1)
    {
        qCritical("Only one detector module is supported.");
        return -1;
    }
    // check dimensions of projections
    auto projImgDims = ioProjImg->metaInfo(fnProjImg).value(CTL::io::meta_info::dimensions).value
                                                           <CTL::io::meta_info::Dimensions>();
    auto nbViews = projImgDims.nbDim < 4 ? projImgDims.dim3 : projImgDims.dim4;
    if(Ps.nbViews() != nbViews)
    {
        qCritical("Number of projection matrices and number of projections do not match.");
        return -1;
    }

    qInfo("Load 3D Radon space...");
    auto radon3d = ioVol->readVolume(fnRadon);

    // Derivative, if required
    if(parser.isSet("d"))
    {
        qInfo("Derivative of 3D Radon space...");
        CTL::imgproc::diff<2>(radon3d);
        radon3d /= radon3d.voxelSize().z; // divide by distance spacing
    }

    // Init resampler
    CTL::OCL::VolumeResampler radonSpaceSampler(radon3d);

    // Init optimizer
    CTL::NLOPT::GrangeatRegistration2D3D reg;

    //metric
    auto gmc = parser.isSet("m")
               ? parser.value("m").toDouble()
               : 50.0;
    auto metric = CTL::imgproc::GemanMcClure(gmc);
    reg.setMetric(&metric);

    // sub-sampling
    auto subSampling = parser.isSet("s")
                       ? parser.value("s").toFloat()
                       : 0.1f;
    if(!parser.isSet("n"))
        reg.setSubSamplingLevel(subSampling);

    // init algorithm
    auto& opt = reg.optObject();

    // global or local
    bool global = parser.isSet("g");
    if(global)
    {
        opt = nlopt::opt{ nlopt::algorithm::GN_CRS2_LM, 6u };
        opt.set_maxtime(1000.0);
        opt.set_population(500u);
    }
    else
    {
        reg.optObject().set_xtol_rel(-1.0);
        reg.optObject().set_initial_step({ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 });
    }
    reg.optObject().set_xtol_abs(0.001);

    // set bounds
    opt.set_lower_bounds(parser.isSet("l")
                         ? parser.value("l").toDouble()
                         : -30.0);
    opt.set_upper_bounds(parser.isSet("u")
                         ? parser.value("u").toDouble()
                         : 30.0);

    // Optimization
    qInfo("Start optimization...");
    QVector<double> rot, transl;
    QElapsedTimer time; time.start();

    for(uint v = 0; v < nbViews; ++v)
    {
        auto proj = projDataIO(fnProjImg)->readSingleView(fnProjImg, v, 1).module(0);
        auto& pMat = Ps.view(v).module(0);

        auto optHomo = reg.optimize(proj, radonSpaceSampler, pMat);
        auto rotAxis = CTL::mat::rotationAxis(optHomo.subMat<0,2, 0,2>());
        auto translVec = optHomo.subMat<0,2, 3,3>();

        qInfo().noquote() << "\n" + QString::fromStdString(rotAxis.info());
        qInfo().noquote() << "\n" + QString::fromStdString(translVec.info());

        rot.append(CTL::mat::toQVector(rotAxis));
        transl.append(CTL::mat::toQVector(translVec));
    }
    qInfo() << "Elapsed time [ms]:" << time.elapsed();

    // Save result
    QString outPath = ".";
    if(parser.isSet("o"))
    {
        outPath = parser.value("o");
        if(outPath.right(1) == "/" || outPath.right(1) == "\\")
            outPath.chop(1);
    }

    QString suffix = (global ? QString("_global") : QString("_local")) +
                     "_sub" + (parser.isSet("n") ? QString("1.0") : QString::number(subSampling)) +
                     "_gmc" + QString::number(gmc) +
                     ".den";
    CTL::io::den::save(rot, outPath + "/reg_rot" + suffix, nbViews, 3);
    CTL::io::den::save(transl, outPath + "/reg_trans" + suffix, nbViews, 3);

    return 0;
}

std::unique_ptr<CTL::io::AbstractProjectionMatrixIO> projMatIO(const QString& fnProjMat)
{
    using namespace CTL::io;
    using NrrdIO = BaseTypeIO<NrrdFileIO>;
    using DenIO = BaseTypeIO<DenFileIO>;

    std::unique_ptr<AbstractProjectionMatrixIO> ioProjMat;

    auto fnProjMatSuffix = fnProjMat.split(".").last();

    if(fnProjMatSuffix.compare("den", Qt::CaseInsensitive) == 0)
    {
        ioProjMat = DenIO::makeProjectionMatrixIO();
    }
    else // assume NRRD
    {
        ioProjMat = NrrdIO::makeProjectionMatrixIO();
        if(!ioProjMat->metaInfo(fnProjMat).contains("nrrd version")) // fall-back to DEN
        {
            qInfo("Assume DEN input format for projection matrices.");
            ioProjMat = DenIO::makeProjectionMatrixIO();
        }
    }

    return ioProjMat;
}

std::unique_ptr<CTL::io::AbstractProjectionDataIO> projDataIO(const QString& fnProjImg)
{
    using namespace CTL::io;
    using NrrdIO = BaseTypeIO<NrrdFileIO>;
    using DenIO = BaseTypeIO<DenFileIO>;

    std::unique_ptr<AbstractProjectionDataIO> ioProjImg;

    auto fnProjImgSuffix = fnProjImg.split(".").last();

    if(fnProjImgSuffix.compare("den", Qt::CaseInsensitive) == 0)
    {
        ioProjImg = DenIO::makeProjectionDataIO();
    }
    else // assume NRRD
    {
        ioProjImg = NrrdIO::makeProjectionDataIO();
        if(!ioProjImg->metaInfo(fnProjImg).contains("nrrd version")) // fall-back to DEN
        {
            qInfo("Assume DEN input format for projection images.");
            ioProjImg = DenIO::makeProjectionDataIO();
        }
    }

    return ioProjImg;
}

std::unique_ptr<CTL::io::AbstractVolumeIO<float>> volumeIO(const QString& fnRadon)
{
    using namespace CTL::io;
    using NrrdIO = BaseTypeIO<NrrdFileIO>;

    std::unique_ptr<AbstractVolumeIO<float>> ioVol = NrrdIO::makeVolumeIO<float>();

    if(!ioVol->metaInfo(fnRadon).contains("nrrd version"))
    {
        qCritical("Only NRRD format is supported for pre-computed 3D Radon space.");
        return nullptr;
    }

    return ioVol;
}

