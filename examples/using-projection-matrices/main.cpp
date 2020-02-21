/*
 * This minimal example shows how to project a volume by using projection matrices.
 * Volume and projection matrices are loaded from DEN files.
 */

#include "acquisition/geometrydecoder.h"
#include "io/den/denfileio.h"
#include "projectors/raycasterprojector.h"
#include <QCoreApplication>

const QString IN_VOLUME_PATH = "path/to/volume.den";
const QString IN_PROJECTION_MATRICES_PATH = "path/to/projection_matrices.den";
const QString OUT_PROJECTIONS_PATH = "path/to/projection_output.den";

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    try
    {
        // IO object for DEN files
        CTL::io::BaseTypeIO<CTL::io::DenFileIO> io;

        // load volume
        auto volume = io.readVolume<float>(IN_VOLUME_PATH);
        volume.setVoxelSize(1.0f, 1.0f, 1.0f); // not encoded in DEN file format

        // load projection matrices describing a full scan geometry
        uint nbModules = 1; // single flat-panel detector
        auto projMats = io.readFullGeometry(IN_PROJECTION_MATRICES_PATH, nbModules);

        // decode an acquisition setup
        QSize nbDetectorPixelsPerModule = { 640, 480 };
        auto setup = CTL::GeometryDecoder::decodeFullGeometry(projMats, nbDetectorPixelsPerModule);

        // configure a projector and project volume
        CTL::OCL::RayCasterProjector projector;  // default config (change via projector.settings())
        projector.configure(setup);
        auto projections = projector.project(volume);

        // save projections
        io.write(projections, OUT_PROJECTIONS_PATH);
    }
    catch(const std::exception& e)
    {
        std::cerr << "exception caught:\n" << e.what() << std::endl;
        return -1;
    }

    std::cout << "end of program" << std::endl;
    return 0;
}
