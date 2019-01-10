CTL - Computed Tomography Library
=================================

[**Check out the documentation!**](https://web.stimulate.ovgu.de/abtheo/doc/html/index.html)

The following installation guide has been tested with Kubuntu 18.04 LTS.

Install Qt
----------

1. Qt libraries:

    ~~~~~~~~~~~~~{.sh}
    sudo apt install qt5-default
    ~~~~~~~~~~~~~
    
2. Qt Creator (optional but recommended):

    ~~~~~~~~~~~~~{.sh}
    sudo apt install qtcreator
    ~~~~~~~~~~~~~

Install OpenCL
--------------

This is an example how to set up OpenCL for an NVIDIA GPU.

1. install official NVIDIA driver using Driver Manager (tested with driver version 390.48 + GTX1080 Ti) --> reboot
    
2. install clinfo (optional)

    ~~~~~~~~~~~~~{.sh}
    sudo apt install clinfo
    ~~~~~~~~~~~~~
    Then, you can just type
    ~~~~~~~~~~~~~{.sh}
    clinfo
    ~~~~~~~~~~~~~
    in order to get a summary of you OpenCL devices.
    
3. install Nvidia OpenCL dev stuff

    ~~~~~~~~~~~~~{.sh}
    sudo apt install nvidia-opencl-dev
    ~~~~~~~~~~~~~

4. get OpenCL headers (if not already there)

    ~~~~~~~~~~~~~{.sh}
    sudo apt install opencl-headers
    ~~~~~~~~~~~~~

No matter what kind of OpenCL device you want to use, at the end of the day
there needs to be at least a symbolic link *libOpenCL.so* in the library folder
(/usr/lib/) that points to a valid ICD loader.

Compile a project
-----------------

~~~~~~~~~~~~~{.sh}
cd /path/to/source (where the .pro file lives)
mkdir build
cd build
qmake ..
make
~~~~~~~~~~~~~

Teaser: Making projections
--------------------------

~~~~~~~~~~~~~{.cpp}
#include "acquisition/acquisitionsetup.h"
#include "acquisition/systemblueprints.h"
#include "acquisition/trajectories.h"
#include "io/den/denfileio.h"
#include "mat/mat.h"
#include "projectors/raycasterprojector.h"
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    // IO object (reads/writes basic types) with a certain IO type
    // as a template argument - here for DEN files
    CTL::io::BaseTypeIO<CTL::io::DenFileIO> io;

    // volume
    auto volume = io.readVolume<float>("path/to/volume.den");
    volume.setVoxelSize(1.0f, 1.0f, 1.0f); // not encoded in DEN files

    // use of a predef system
    auto system = CTL::CTsystemBuilder::createFromBlueprint(
        CTL::blueprints::GenericCarmCT(CTL::DetectorBinning::Binning4x4));

    // create an acquisition setup
    CTL::AcquisitionSetup myCarmSetup(system);
    myCarmSetup.addPreparationProtocol(CTL::protocols::WobbleTrajectory(300, 200.0_deg, 750.0));
    if(!myCarmSetup.isValid())
        return -1;

    // configure a projector and project volume
    CTL::OCL::RayCasterProjector::Config rcConfig;  // config with standard settings
    CTL::OCL::RayCasterProjector myProjector;       // the projector
    myProjector.configurate(myCarmSetup, rcConfig); // configurate step
    auto projections = myProjector.project(volume); // project
    
    // save projections
    io.write(projections, "path/to/projections.den");
    return 0;
}

/*
 * NOTE: the project file (.pro) needs to include the following modules to be
 * able to compile this example program:
 *
 *  include(path/to/ctl.pri)
 *  include(path/to/den_file_io.pri)
 *  include(path/to/ocl_config.pri)
 *  include(path/to/ocl_projectors.pri)
 */
 ~~~~~~~~~~~~~
