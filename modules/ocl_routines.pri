# dependency
!OCL_CONFIG_MODULE: error("OCL_ROUTINES_MODULE needs OCL_CONFIG_MODULE -> include ocl_config.pri before ocl_routines.pri")
!CTL_CORE_MODULE: error("OCL_ROUTINES_MODULE needs CTL_CORE_MODULE -> include ctl.pri before ocl_routines.pri")

# declare module
CONFIG += OCL_ROUTINES_MODULE

HEADERS += \
    $$PWD/src/projectors/raycaster.h \
    $$PWD/src/projectors/raycasteradapter.h \
    $$PWD/src/projectors/raycasterprojector.h \
    $$PWD/src/processing/volumeslicer.h \
    $$PWD/src/processing/radontransform3d.h \
    $$PWD/src/processing/radontransform2d.h

SOURCES += \
    $$PWD/src/projectors/raycaster.cpp \
    $$PWD/src/projectors/raycasteradapter.cpp \
    $$PWD/src/projectors/raycasterprojector.cpp \
    $$PWD/src/processing/volumeslicer.cpp \
    $$PWD/src/processing/radontransform3d.cpp \
    $$PWD/src/processing/radontransform2d.cpp

# OpenCL source files
DISTFILES += \
    $$PWD/src/ocl/cl_src/projectors/external_raycaster.cl \
    $$PWD/src/ocl/cl_src/projectors/raycasterprojector_interp.cl \
    $$PWD/src/ocl/cl_src/projectors/raycasterprojector_no_interp.cl \
    $$PWD/src/ocl/cl_src/processing/volumeSlicer.cl \
    $$PWD/src/ocl/cl_src/processing/planeIntegral.cl \
    $$PWD/src/ocl/cl_src/processing/radon2d.cl
