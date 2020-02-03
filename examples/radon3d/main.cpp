#include "io/den/denfileio.h"
#include "io/nrrd/nrrdfileio.h"
#include "mat/mat.h"
#include "processing/coordinates.h"
#include "processing/diff.h"
#include "processing/radontransform3d.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QDebug>

enum ReturnValue { Success, WrongSyntax, InvalidValue, FileIOError };
static ReturnValue printCriticalMessage(ReturnValue retVal, const QString &msg);
static QString hash(const CTL::VoxelVolume<float>& vol);
static std::unique_ptr<CTL::io::AbstractVolumeIO<float>> volumeIO(const QString& fn, bool voxSizeParsed);
static CTL::VoxelVolume<float> mirrorRadonSpace(CTL::VoxelVolume<float>&& radon3dFirstPart);

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationVersion("1.5");

    QCommandLineParser parser;
    parser.setApplicationDescription("3D Radon Transform + Derivative");
    parser.addVersionOption();
    parser.addHelpOption();

    parser.addPositionalArgument("source", "Input file path to CT volume [NRRD/DEN].");

    parser.addOptions({
        { { "x", "voxel-size-x" }, "X size of voxels [mm].", "size" },
        { { "y", "voxel-size-y" }, "Y size of voxels [mm].", "size" },
        { { "z", "voxel-size-z" }, "Z size of voxels [mm].", "size" },
        { { "i", "isotropic-voxel-size" }, "Isotropic voxels size [mm].", "size" },

        { { "n", "number-angles" }, "Number of samples for azimuth and polar angles.", "number" },
        { { "a", "number-azimuth-samples" }, "Number of samples for azimuth angles.", "number" },
        { { "p", "number-polar-samples" }, "Number of samples for polar angles.", "number" },

        { { "0", "no-diff" }, "Omit derivative (only 3D Radon transform)." },

        { { "j", "device-number" }, "Use only a specific OpenCL device with index 'number'.", "number" },

        { "o", "Output file name.", "path" }
    });

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if(args.size() != 1)
        return printCriticalMessage(WrongSyntax, "Wrong syntax. Check help with -h or --help.");

    if(parser.isSet("i") && (parser.isSet("x") || parser.isSet("y") || parser.isSet("z")))
        return printCriticalMessage(WrongSyntax, "i and x|y|z flag are mutually exclusive.");

    if(parser.isSet("n") && (parser.isSet("a") || parser.isSet("p")))
        return printCriticalMessage(WrongSyntax, "n and a|p flag are mutually exclusive.");

    if(parser.isSet("j"))
        CTL::OCL::OpenCLConfig::instance().setDevices( {
            CTL::OCL::OpenCLConfig::instance().devices().at(parser.value("j").toUInt()) } );

    // figure out the file format
    auto fnIn = args.at(0);
    const bool voxSizeParsed = (parser.isSet("x") && parser.isSet("y") && parser.isSet("z")) ||
                               parser.isSet("i");
    auto io = volumeIO(fnIn, voxSizeParsed);
    if(!io)
        return printCriticalMessage(FileIOError, "Invalid input file.");

    // load volume
    qInfo("Load volume...");
    auto vol = io->readVolume(fnIn);
    if(vol.allocatedElements() == 0u)
        return printCriticalMessage(FileIOError, "No data has been loaded.");

    // voxel size
    auto voxSize = vol.voxelSize();
    if(parser.isSet("i"))
    {
        auto isoSize = parser.value("i").toFloat();
        voxSize = { isoSize, isoSize, isoSize };
    }
    if(parser.isSet("x")) voxSize.x = parser.value("x").toFloat();
    if(parser.isSet("y")) voxSize.y = parser.value("y").toFloat();
    if(parser.isSet("z")) voxSize.z = parser.value("z").toFloat();
    vol.setVoxelSize(voxSize);

    // ranges
    const auto volDiag = std::sqrt(voxSize.x*vol.nbVoxels().x * voxSize.x*vol.nbVoxels().x +
                                   voxSize.y*vol.nbVoxels().y * voxSize.y*vol.nbVoxels().y +
                                   voxSize.z*vol.nbVoxels().z * voxSize.z*vol.nbVoxels().z);
    CTL::SamplingRange phiRange{ -180.0_deg, 180.0_deg };
    const CTL::SamplingRange thetaRange{ 0.0_deg, 180.0_deg };
    const CTL::SamplingRange distRange{ -volDiag / 2.0f, volDiag / 2.0f };
    const CTL::SamplingRange distRangeFirstHalf{ -volDiag / 2.0f, 0.0f };

    // number samples for distances
    auto nbDist = uint(volDiag / std::min(std::min(voxSize.x, voxSize.y), voxSize.z));
    if(nbDist % 2 == 0)
        ++nbDist;
    // number samples for angles (azimuth and polar)
    const auto nbAngles = parser.isSet("n") ? parser.value("n").toUInt() : nbDist;
    const auto nbPhi    = parser.isSet("a") ? parser.value("a").toUInt() : nbAngles;
    const auto nbTheta  = parser.isSet("p") ? parser.value("p").toUInt() : nbAngles;
    if(nbPhi % 2 == 0)
        phiRange.end() += phiRange.spacing(nbPhi - 1);

    if(nbPhi < 3 || nbTheta < 3 || nbDist < 3)
        return printCriticalMessage(InvalidValue, "Number of samples for the 3D Radon space must be"
                                    " at least three in each dimension.");

    // radon transform
    qInfo() << "Compute 3D Radon space with ( azi, polar, distance ) = ("
            << nbPhi << "," << nbTheta << "," << nbDist << ") samples...";
    CTL::OCL::RadonTransform3D radon3d(vol);
    auto radon3dFirstPart = radon3d.sampleTransform(phiRange, nbPhi,
                                                    thetaRange, nbTheta,
                                                    distRangeFirstHalf, nbDist / 2 + 1);

    // mirror second half of 3D Radon space
    auto radon3DAugmented = mirrorRadonSpace(std::move(radon3dFirstPart));

    // Derivative
    if(!parser.isSet("0"))
    {
        qInfo("Derivative of 3D Radon space...");
        CTL::imgproc::diff<2>(radon3DAugmented);
        radon3DAugmented /= radon3DAugmented.voxelSize().z; // divide by distance spacing
    }

    // save result
    qInfo("Write output file...");
    QString fnOut{ fnIn + "_radon3d.nrrd" };
    if(parser.isSet("o")) fnOut = parser.value("o");

    QVariantMap metaInfo{
        { "dim1 range - azimuth angle [rad]",
          "[" + QString::number(phiRange.start()) + "," + QString::number(phiRange.end()) + "]"},
        { "dim2 range - polar angle [rad]",
          "[" + QString::number(thetaRange.start()) + "," + QString::number(thetaRange.end()) + "]" },
        { "dim3 range - distance [mm]",
          "[" + QString::number(distRange.start()) + "," + QString::number(distRange.end()) + "]" },
        { "input volume hash", hash(vol) },
        { "what", "3D Radon transform (radon3d version " + QCoreApplication::applicationVersion() + ")" }
                        };

    if(!CTL::io::BaseTypeIO<CTL::io::NrrdFileIO>{}.write(radon3DAugmented, fnOut, metaInfo))
        return printCriticalMessage(FileIOError, "Unable to write output.");

    return ReturnValue::Success;
}

QString hash(const CTL::VoxelVolume<float>& vol)
{
    QCryptographicHash hash(QCryptographicHash::Md5);

    // hash value from volume properties
    QByteArray bufferProperties;

    bufferProperties.append(QString::number(vol.nbVoxels().x));
    bufferProperties.append(QString::number(vol.nbVoxels().y));
    bufferProperties.append(QString::number(vol.nbVoxels().z));

    bufferProperties.append(QString::number(vol.voxelSize().x));
    bufferProperties.append(QString::number(vol.voxelSize().y));
    bufferProperties.append(QString::number(vol.voxelSize().z));

    bufferProperties.append(QString::number(vol.offset().x));
    bufferProperties.append(QString::number(vol.offset().y));
    bufferProperties.append(QString::number(vol.offset().z));

    hash.addData(bufferProperties);

    // has value from data (simple custom hash function)
    constexpr auto valSize = sizeof(float);
    // initialize data hash to zero
    char dataHash[valSize] = { 0 };
    // help buffer to fetch a value and punning from a float to char array
    char bufferFloat[valSize];

    for(auto val : vol.data())
    {
        // copy bit pattern of one value
        std::memcpy(&bufferFloat, &val, valSize);

        // combine data hash with bit pattern of the value
        auto ptrToDataHash = dataHash;
        for(auto v : bufferFloat)
            *ptrToDataHash++ += 3 * v;
    }

    hash.addData(dataHash, valSize);

    return hash.result().toHex();
}

std::unique_ptr<CTL::io::AbstractVolumeIO<float>> volumeIO(const QString& fnIn, bool voxSizeParsed)
{
    using NrrdIO = CTL::io::BaseTypeIO<CTL::io::NrrdFileIO>;
    using DenIO = CTL::io::BaseTypeIO<CTL::io::DenFileIO>;

    std::unique_ptr<CTL::io::AbstractVolumeIO<float>> io;

    const auto fnInSuffix = fnIn.split(".").last();


    if(fnInSuffix.compare("den", Qt::CaseInsensitive) == 0)
    {
        if(!voxSizeParsed)
        {
            qCritical("Error: voxel sizes need to be specified.");
            return nullptr;
        }
        io = DenIO::makeVolumeIO<float>();
    }
    else // assume NRRD
    {
        io = NrrdIO::makeVolumeIO<float>();
        if(!io->metaInfo(fnIn).contains("nrrd version")) // fall-back to DEN
        {
            qInfo("Assume DEN input format for volume input.");
            if(!voxSizeParsed)
            {
                qCritical("Error: voxel sizes need to be specified.");
                return nullptr;
            }
            io = DenIO::makeVolumeIO<float>();
        }
    }

    return io;
}

CTL::VoxelVolume<float> mirrorRadonSpace(CTL::VoxelVolume<float>&& radon3dFirstPart)
{
    const uint nbPhi = radon3dFirstPart.dimensions().x;
    const uint nbTheta = radon3dFirstPart.dimensions().y;
    const uint nbDist = radon3dFirstPart.dimensions().z * 2 - 1;

    // defensive check
    if(nbPhi < 3u)
    {
        qCritical() << "Error: something went wrong. Number of azimuth samples:" << nbPhi;
        return CTL::VoxelVolume<float>{ 0, 0, 0, -1.0f, -1.0f, -1.0f };
    }

    // steal and augment data
    auto augmentedData = std::move(radon3dFirstPart.data());
    augmentedData.resize(nbPhi * size_t(nbTheta) * nbDist);

    // create augmented volume and initialize with the first half
    CTL::VoxelVolume<float> ret(nbPhi, nbTheta, nbDist, std::move(augmentedData));
    // cpy voxel size
    ret.setVoxelSize(radon3dFirstPart.voxelSize());
    // reset volume offset
    auto offset = radon3dFirstPart.offset();
    offset.z = 0.0f; // distance range is now centered
    ret.setVolumeOffset(offset);

    // helper for phi sampling
    const auto sampleOf2Pi = (nbPhi % 2) ? (nbPhi - 1) : (nbPhi - 2);
    const auto sampleOfPi = sampleOf2Pi / 2;

    // fill second half (phi, theta, -d) -> ((phi+pi) mod 2pi, pi-theta, d)
    for(auto d = (nbDist / 2) + 1,
             dNeg = (nbDist / 2) - 1;
        d < nbDist; ++d, --dNeg)

        for(auto theta = 0u,
                 thetaMirr = nbTheta - 1;
            theta < nbTheta; ++theta, --thetaMirr)

            for(auto phi = 0u,
                     phiShift = sampleOfPi;
                phi < nbPhi; ++phi, ++phiShift)

                ret(phi, theta, d) = ret(phiShift % sampleOf2Pi, thetaMirr, dNeg);

    return ret;
}

ReturnValue printCriticalMessage(ReturnValue retVal, const QString& msg)
{
    qCritical().noquote() << "Error:" << msg;
    return retVal;
}
