# dependency
!OCL_CONFIG_MODULE: error("OCL_ROUTINES_MODULE needs OCL_CONFIG_MODULE -> include ocl_config.pri before ocl_routines.pri")
!CTL_CORE_MODULE: error("OCL_ROUTINES_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before ocl_routines.pri")

# declare module
CONFIG += OCL_ROUTINES_MODULE

HEADERS += \
    $$PWD/../src/processing/consistency.h \
    $$PWD/../src/processing/imageresampler.h \
    $$PWD/../src/processing/radontransform2d.h \
    $$PWD/../src/processing/radontransform3d.h \
    $$PWD/../src/projectors/raycaster.h \
    $$PWD/../src/projectors/raycasteradapter.h \
    $$PWD/../src/projectors/raycasterprojector.h \
    $$PWD/../src/processing/volumeresampler.h \
    $$PWD/../src/processing/volumeslicer.h \
    $$PWD/../src/projectors/standardpipeline.h

SOURCES += \
    $$PWD/../src/processing/consistency.cpp \
    $$PWD/../src/processing/imageresampler.cpp \
    $$PWD/../src/processing/radontransform2d.cpp \
    $$PWD/../src/processing/radontransform3d.cpp \
    $$PWD/../src/projectors/raycaster.cpp \
    $$PWD/../src/projectors/raycasteradapter.cpp \
    $$PWD/../src/projectors/raycasterprojector.cpp \
    $$PWD/../src/processing/volumeresampler.cpp \
    $$PWD/../src/processing/volumeslicer.cpp \
    $$PWD/../src/projectors/standardpipeline.cpp


# OpenCL source files
DISTFILES += \
    $$PWD/../src/ocl/cl_src/projectors/external_raycaster.cl \
    $$PWD/../src/ocl/cl_src/projectors/raycasterprojector_interp.cl \
    $$PWD/../src/ocl/cl_src/projectors/raycasterprojector_no_interp.cl \
    $$PWD/../src/ocl/cl_src/processing/imageResampler.cl \
    $$PWD/../src/ocl/cl_src/processing/planeIntegral.cl \
    $$PWD/../src/ocl/cl_src/processing/radon2d.cl \
    $$PWD/../src/ocl/cl_src/processing/volumeResampler.cl \
    $$PWD/../src/ocl/cl_src/processing/volumeSlicer.cl
