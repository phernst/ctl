# dependency
!OCL_CONFIG_MODULE: error("OCL_PROJECTORS_MODULE needs OCL_CONFIG_MODULE -> include ocl_config.pri before ocl_projectors.pri")
!CTL_CORE_MODULE: error("OCL_PROJECTORS_MODULE needs CTL_CORE_MODULE -> include ctl.pri before ocl_projectors.pri")

# declare module
CONFIG += OCL_PROJECTORS_MODULE

HEADERS += \
    $$PWD/src/projectors/raycaster.h \
    $$PWD/src/projectors/raycasteradapter.h \
    $$PWD/src/projectors/raycasterprojector.h

SOURCES += \
    $$PWD/src/projectors/raycaster.cpp \
    $$PWD/src/projectors/raycasteradapter.cpp \
    $$PWD/src/projectors/raycasterprojector.cpp

# OpenCL source files
DISTFILES += \
    $$PWD/src/ocl/cl_src/projectors/external_raycaster.cl \
    $$PWD/src/ocl/cl_src/projectors/raycasterprojector_interp.cl \
    $$PWD/src/ocl/cl_src/projectors/raycasterprojector_no_interp.cl
