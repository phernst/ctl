# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# dependency
!OCL_CONFIG_MODULE: error("OCL_ROUTINES_MODULE needs OCL_CONFIG_MODULE -> include ocl_config.pri before ocl_routines.pri")
!CTL_CORE_MODULE: error("OCL_ROUTINES_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before ocl_routines.pri")

# declare module
CONFIG += OCL_ROUTINES_MODULE

HEADERS += \
    $$CTL_SOURCE_DIR/processing/consistency.h \
    $$CTL_SOURCE_DIR/processing/imageresampler.h \
    $$CTL_SOURCE_DIR/processing/radontransform2d.h \
    $$CTL_SOURCE_DIR/processing/radontransform3d.h \
    $$CTL_SOURCE_DIR/projectors/raycaster.h \
    $$CTL_SOURCE_DIR/projectors/raycasteradapter.h \
    $$CTL_SOURCE_DIR/projectors/raycasterprojector.h \
    $$CTL_SOURCE_DIR/processing/volumeresampler.h \
    $$CTL_SOURCE_DIR/processing/volumeslicer.h

SOURCES += \
    $$CTL_SOURCE_DIR/processing/consistency.cpp \
    $$CTL_SOURCE_DIR/processing/imageresampler.cpp \
    $$CTL_SOURCE_DIR/processing/radontransform2d.cpp \
    $$CTL_SOURCE_DIR/processing/radontransform3d.cpp \
    $$CTL_SOURCE_DIR/projectors/raycaster.cpp \
    $$CTL_SOURCE_DIR/projectors/raycasteradapter.cpp \
    $$CTL_SOURCE_DIR/projectors/raycasterprojector.cpp \
    $$CTL_SOURCE_DIR/processing/volumeresampler.cpp \
    $$CTL_SOURCE_DIR/processing/volumeslicer.cpp


# OpenCL source files
DISTFILES += \
    $$CTL_SOURCE_DIR/ocl/cl_src/projectors/external_raycaster.cl \
    $$CTL_SOURCE_DIR/ocl/cl_src/projectors/raycasterprojector_interp.cl \
    $$CTL_SOURCE_DIR/ocl/cl_src/projectors/raycasterprojector_no_interp.cl \
    $$CTL_SOURCE_DIR/ocl/cl_src/processing/imageResampler.cl \
    $$CTL_SOURCE_DIR/ocl/cl_src/processing/planeIntegral.cl \
    $$CTL_SOURCE_DIR/ocl/cl_src/processing/radon2d.cl \
    $$CTL_SOURCE_DIR/ocl/cl_src/processing/volumeResampler.cl \
    $$CTL_SOURCE_DIR/ocl/cl_src/processing/volumeSlicer.cl
