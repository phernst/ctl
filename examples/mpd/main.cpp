#include "mat/mat.h"
#include "io/den/denfileio.h"
#include "io/den/den_utils.h"

#include <QCoreApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("mean projection distance (mPD)");
    parser.addVersionOption();
    parser.addHelpOption();

    parser.addPositionalArgument("groundtruth-pmats", "DEN file");
    parser.addPositionalArgument("initial-pmats", "DEN file");
    parser.addPositionalArgument("estimate-rotation", "DEN file");
    parser.addPositionalArgument("estimate-translation", "DEN file");
    parser.addOptions({
        { "o", "Output file name.", "path" }
    });
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if(args.size() != 4)
    {
        qCritical("Wrong syntax. Check help with -h or --help.");
        return -1;
    }

    // load data
    using namespace CTL::io;
    BaseTypeIO<DenFileIO> io;
    auto pMatsGold = io.readFullGeometry(parser.positionalArguments().at(0), 1);
    auto pMatsInit = io.readFullGeometry(parser.positionalArguments().at(1), 1);
    auto rotations = den::loadDouble<den::std_vector>(parser.positionalArguments().at(2));
    auto translations = den::loadDouble<den::std_vector>(parser.positionalArguments().at(3));

    uint nbPmats = pMatsGold.size();

    if(nbPmats != pMatsInit.size() || nbPmats != rotations.size() / 3 || nbPmats != translations.size() / 3)
    {
        qCritical("Stack sizes do not fit.");
        return -1;
    }

    // create estimated projection matrices
    qDebug() << "create estimated projection matrices";
    CTL::FullGeometry pMats;
    auto rotIt = rotations.begin();
    auto transIt = translations.begin();
    std::for_each(pMatsInit.begin(), pMatsInit.end(),
                  [&rotIt, &transIt, &pMats](const CTL::SingleViewGeometry& g)
    {
        const CTL::ProjectionMatrix P =
            g.module(0) * CTL::Homography3D(
                    CTL::mat::rotationMatrix({ rotIt[0], rotIt[1], rotIt[2] }),
                    { transIt[0], transIt[1], transIt[2] });
        pMats.append(CTL::SingleViewGeometry{ { P } });
        rotIt += 3;
        transIt += 3;
    });

    // compute mPD
    qDebug() << "compute mPD";
    CTL::mat::PMatComparator comp;
    comp.setTotalVolumeSize(248.17, 248.17, 190.49);
    comp.setVolumeGridSpacing(1.0, 1.0, 1.0);
    comp.setRestrictionToDetectorArea(false);

    std::vector<double> mPD(nbPmats);
    std::transform(pMatsGold.begin(), pMatsGold.end(), pMats.begin(), mPD.begin(),
                   [&comp](const CTL::SingleViewGeometry& p1, const CTL::SingleViewGeometry& p2)
    {
        auto res = comp(p1.module(0), p2.module(0));
        qDebug() << res.meanError << "(" << res.minError << "..." << res.maxError << ")";
        return res.meanError;
    });

    // save result
    QString fnOut = parser.isSet("o") ? parser.value("o") : "mpd.den";
    CTL::io::den::save(mPD, fnOut, int(nbPmats));

    return 0;
}
