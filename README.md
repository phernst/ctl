CTL - Computed Tomography Library
=================================

**Current version: 0.3.2**

A C++11 toolkit for CT imaging with minimal dependencies.

This early release enables to simluate freely customizable cone-beam X-ray
imaging setups.
Typical settings like helical CT or C-arm CT with curved or flat panel
detector are included as well. Conversion of the geometric information to/from
projection matrices allowes to easily collaborate with other tools supporting 
them.

Reference publication:  
Tim Pfeiffer, Robert Frysch, Richard N. K. Bismark, and Georg Rose
"CTL: modular open-source C++-library for CT-simulations",
Proc. SPIE 11072,
15th International Meeting on Fully Three-Dimensional Image Reconstruction in Radiology and Nuclear Medicine,
110721L (28 May 2019);
[doi:10.1117/12.2534517](https://doi.org/10.1117/12.2534517)

For further details, check
[the documentation of the code.](https://www.forschungscampus-stimulate.de/ctl/)

Moreover, you may find useful information in the
[Wiki](https://gitlab.com/tpfeiffe/ctl/wikis/home),
especially if you want to contribute to the project as a developer.

In case you are looking for some Python bindings check out [PyCTL](https://github.com/phernst/pyctl) by Philipp Ernst.

The following installation guide has been tested with Kubuntu 18.04 LTS.  
You may also try a preconfigured Docker image. In this case, you can skip the 
following three steps and go directly to *Use a Docker image*.

Install compiler and build tools (GCC, make, ...)
---------------------------------------------------

```console
sudo apt install build-essential
```

Install Qt
----------

The [Qt Core module](https://doc.qt.io/qt-5/qtcore-index.html) is the only
library required for the main module
"[CTL core](https://gitlab.com/tpfeiffe/ctl/blob/master/modules/ctl.pri)" of this
project.

1. Qt libraries

    ```console
    sudo apt install qt5-default
    ```
    
2. Qt3D (optional, for
[3D GUI widgets](https://gitlab.com/tpfeiffe/ctl/blob/master/modules/submodules/gui_widgets_3d.pri))
    
    ```console
    sudo apt install qt3d5-dev
    ```
    If you are on another platform, note that a Qt3D version is needed that 
    requires a Qt version >= 5.9.
3. QtCharts (optional, for [chart-based widgets](https://gitlab.com/tpfeiffe/ctl/blob/dev_v0.3.2/modules/submodules/gui_widgets_charts.pri))
    ```console
    sudo apt install libqt5charts5-dev
    ```

Install OpenCL 1.1/1.2
----------------------

This is an example how to set up
[OpenCL](https://github.khronos.org/OpenCL-CLHPP/) for a NVIDIA GPU. OpenCL and
its C++-API is required for the
[CTL OCL](https://gitlab.com/tpfeiffe/ctl/blob/master/modules/ctl_ocl.pri)
module.

1. install official NVIDIA driver using Driver Manager
(tested with driver version 390.48 + GTX1080 Ti) --> reboot

2. install Nvidia OpenCL development package

    ```console
    sudo apt install nvidia-opencl-dev
    ```
    For other vendors
    ([AMD](https://linuxconfig.org/install-opencl-for-the-amdgpu-open-source-drivers-on-debian-and-ubuntu),
    [Intel](https://software.intel.com/en-us/intel-opencl/download),
    ...),  you need to search for an appropriate
    package that provides you an OpenCL driver.
    
3. install clinfo (optional, for checking available OpenCL devices)

    ```console
    sudo apt install clinfo
    ```
    Then, you can just type
    ```console
    clinfo
    ```
    in order to get a summary of your OpenCL devices.
    You can make sure that the device you intend to use appears in the list.

4. install OpenCL headers (might be already there)

    ```console
    sudo apt install opencl-headers
    ```

No matter what kind of OpenCL device you want to use, you should end up with a 
*libOpenCL.so* (usually symbolic link) in the library folder
(/usr/lib/) that points to a valid ICD loader. The ICD loader should have been
installed with your vendor driver. If this has not happend you will not be able
to link against it (linker error when using `LIBS += -lOpenCL` in your qmake
project file). Then, you can install an ICD loader by your own, e.g. the package
*mesa-opencl-icd*.

It might happen on some Debian platforms, that the *CL/cl.hpp* header is missing
even when you have installed the 'opencl-headers' package. In this case, you can
manually download the header file from the
[khronos group](https://www.khronos.org/registry/OpenCL/api/2.1/cl.hpp).

Use a Docker image
------------------

Instead of installing the above libraries, you may also start directly by using
a Docker image.
There are two prepared Docker images that you can use as a development/testing
environment:
 * `frysch/ubuntu:ocl-nvidia` for Nvidia GPUs
 * `frysch/ubuntu:ocl-intel` for Intel CPUs

### Run a GUI example inside Docker
Give Docker the rights to access the X-Server:
```console
xhost +local:docker
```
Run Docker in interactive mode (`-it`) and shared X11 socket (
`-e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix` - required only for 
showing GUIs):
```console
docker run -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix frysch/ubuntu:ocl-intel bash
```
Inside the Docker container run
```console
git clone https://gitlab.com/tpfeiffe/ctl.git
cd ctl/examples/simTool/
qmake && make -j 4
./bin/SimulationTool
```
in order to compile and run the GUI example "SimulationTool".


For further details about the Docker images, see
[this Wiki page](https://gitlab.com/tpfeiffe/ctl/wikis/Docker-Images).


Develop your own C++ apps or modules
====================================

Use modules that you need
-------------------------

The CTL provides several modules. According to your needs, you can select only a
subset of modules. Each module has a corresponding .pri file that you can
include into your qmake project (.pro file) using the syntax
`include(example_module.pri)` (see also the *examples* or *testing* folder).

The following modules are available:
 * ctl.pri: the core library and file IO
 * ctl_ocl.pri: OpenCL routines and OpenCL environment
 * ctl_qtgui.pri: widgets for visualization purposes
 * ctl_nlopt.pri: NLopt-dependent submodules; currently, Grangeat-based 2D/3D registration


Compile a project
-----------------

```console
cd /path/to/source/where/the/.pro/file/lives
mkdir build
cd build
qmake ..
make
```

Teaser: Making projections
--------------------------

The following example code uses a predefined C-arm system and a predefined
trajectory (a trajectory is a specific acquisition protocol) in order to
project a volume, which is read from a file. This serves to show how the CTL
may work out of the box. However, CT systems or acquisition protocols (or even
preparations of single views) can be freely configured. Moreover, certain
projector extensions can "decorate" the used forward projector in order to
include further geometric/physical/measuring effects.

```cpp
#include "ctl.h"
#include "ctl_ocl.h"
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
 ```
 A corresponding qmake project for this example can be found in
 [examples/readme-example](https://gitlab.com/tpfeiffe/ctl/tree/master/examples/readme-example).

--------------

Statistics on the master branch:

![](https://tokei.rs/b1/GitLab/tpfeiffe/ctl?category=code)
![](https://tokei.rs/b1/GitLab/tpfeiffe/ctl?category=comments)
![](https://tokei.rs/b1/GitLab/tpfeiffe/ctl?category=files)

For more details click [this link](https://rf.p7.de/apps/cms_pico/pico/ctlstat/).

--------------

If you have any problems or questions regarding the CTL, please contact us:  
<sw4g.production@gmail.com>.
