#include "acquisition/acquisitionsetup.h"
#include "acquisition/systemblueprints.h"
#include "acquisition/trajectories.h"
#include "io/nrrd/nrrdfileio.h"
#include "mat/mat.h"
#include "projectors/raycasterprojector.h"
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    // IO object (reads/writes basic types) with a certain IO type
    // as a template argument - here for DEN files
    CTL::io::BaseTypeIO<CTL::io::NrrdFileIO> io;

    // load volume
    auto volume = io.readVolume<float>("path/to/volume.nrrd");

    // use of a predefined system from "acquisition/systemblueprints.h"
    auto system = CTL::CTsystemBuilder::createFromBlueprint(CTL::blueprints::GenericCarmCT());

    // create an acquisition setup
    uint nbViews = 100;
    CTL::AcquisitionSetup myCarmSetup(system, nbViews);
    // add a predefined trajectory to the setup from "acquisition/trajectories.h"
    double angleSpan = 200.0_deg; // floating-point literal _deg from "mat/mat.h" to convert to rad
    double sourceToIsocenter = 750.0; // mm is the standard unit for length dimensions
    myCarmSetup.applyPreparationProtocol(CTL::protocols::WobbleTrajectory(angleSpan,
                                                                          sourceToIsocenter));
    if(!myCarmSetup.isValid())
        return -1;

    // configure a projector and project volume
    CTL::OCL::RayCasterProjector::Config rcConfig;  // config with standard settings
    CTL::OCL::RayCasterProjector myProjector;       // the projector
    myProjector.configure(myCarmSetup, rcConfig);   // configure projector
    auto projections = myProjector.project(volume); // project

    // save projections
    io.write(projections, "path/to/projections.nrrd");

    std::cout << "end of program" << std::endl;
    return 0;
}

/*
 * NOTE: the project file (.pro) needs to include the following modules to be
 * able to compile this example program:
 *
 *  include(path/to/ctl.pri)
 *  include(path/to/nrrd_file_io.pri)
 *  include(path/to/ocl_config.pri)
 *  include(path/to/ocl_projectors.pri)
 */
