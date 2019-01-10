CTL
==============

Computed Tomography Library

[**Check out the documentation!**](https://web.stimulate.ovgu.de/abtheo/doc/html/index.html)

The following installation guide has been tested with Kubuntu 18.04 LTS.

Install Qt
----------

1. Qt libraries:

    `sudo apt install qt5-default`
    
2. Qt Creator (optional but recommended):

    `sudo apt install qtcreator`


Install OpenCL
--------------

### example: set up NVIDIA OpenCL

1. install official NVIDIA driver using Driver Manager (tested with driver version 390.48 + GTX1080 Ti) --> reboot
    
2. install clinfo (optional)

    `sudo apt install clinfo`
    
    Then, you can just type

    `clinfo`
    
    in order to get a summary of you OpenCL devices.
    
3. get OpenCL headers

    `sudo apt install opencl-headers`
    
4. install Nvidia OpenCL dev stuff

    `sudo apt install nvidia-opencl-dev`
    
No matter what kind of OpenCL device you want to use, at the end of the day
there needs to be at least a symbolic link *libOpenCL.so* in the library folder
(/usr/lib/) that points to a valid ICD loader.

### the use of OpenCL libraries in code
in Qt (.pro file):

    LIBS += -lOpenCL

in source code:

    #include <CL/cl.hpp>
    
### compile documentation

in order to compile the docs 

1. install Doxygen

    `sudo apt install doxygen`

2. be able to create the class hierarchy graphs

    `sudo apt install graphviz`

3. go to the source directory and type

    `doxygen Doxyfile`


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
