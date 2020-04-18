#include "acquisition/systemblueprints.h"
#include "acquisition/trajectories.h"
#include "io/nrrd/nrrdfileio.h"
#include "projectors/raycasterprojector.h"
#include <QCoreApplication>
#include <iostream>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    try
    {
        // IO object (reads/writes basic types) with a certain IO type
        // as a template argument - here for NRRD files
        CTL::io::BaseTypeIO<CTL::io::NrrdFileIO> io;

        // load volume
        auto volume = io.readVolume<float>("path/to/volume.nrrd");

        // use of a predefined system from "acquisition/systemblueprints.h"
        auto system = CTL::CTSystemBuilder::createFromBlueprint(CTL::blueprints::GenericCarmCT());

        // create an acquisition setup
        uint nbViews = 100;
        CTL::AcquisitionSetup myCarmSetup(system, nbViews);
        // add a predefined trajectory to the setup from "acquisition/trajectories.h"
        double angleSpan = 200.0_deg; // floating-point literal _deg in "mat/deg.h" converts to rad
        double sourceToIsocenter = 750.0; // mm is the standard unit for length dimensions
        myCarmSetup.applyPreparationProtocol(CTL::protocols::WobbleTrajectory(angleSpan,
                                                                              sourceToIsocenter));
        if(!myCarmSetup.isValid())
            return -1;

        // configure a projector and project volume
        CTL::OCL::RayCasterProjector myProjector;       // the projector (uses its default settings)
        myProjector.configure(myCarmSetup);             // configure projector
        auto projections = myProjector.project(volume); // project

        // save projections
        io.write(projections, "path/to/projections.nrrd");
    }
    catch(const std::exception& e)
    {
        std::cerr << "exception caught:\n" << e.what() << std::endl;
        return -1;
    }

    std::cout << "end of program" << std::endl;
    return 0;
}

/*
 * NOTE: the project file (.pro) needs to include the following modules to be
 * able to compile this example program:
 *
 *  include(path/to/ctl.pri)
 *  include(path/to/ctl_ocl.pri)
 */
