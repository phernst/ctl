# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# dependency
!CTL_CORE_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before grangeat_2d3d_regist.pri")
!OCL_CONFIG_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs OCL_CONFIG_MODULE -> include ocl_config.pri before grangeat_2d3d_regist.pri")
!OCL_ROUTINES_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs OCL_ROUTINES_MODULE -> include ocl_routines.pri before grangeat_2d3d_regist.pri")

# declare module
CONFIG += GRANGEAT_2D3D_REGIST_MODULE

HEADERS += $$CTL_SOURCE_DIR/app/registration/grangeatregistration2d3d.h

SOURCES += $$CTL_SOURCE_DIR/app/registration/grangeatregistration2d3d.cpp

# NLopt library [ https://github.com/stevengj/nlopt/ ]
unix:LIBS += -L/usr/local/lib
LIBS += -lnlopt
