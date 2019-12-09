# dependency
!CTL_CORE_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs CTL_CORE_MODULE -> include ctl.pri before grangeat-2d3d-regist.pri")
!OCL_CONFIG_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs OCL_CONFIG_MODULE -> include ocl_config.pri before grangeat-2d3d-regist.pri")
!OCL_ROUTINES_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs OCL_ROUTINES_MODULE -> include ocl_routines.pri before grangeat-2d3d-regist.pri")

# declare module
CONFIG += GRANGEAT_2D3D_REGIST_MODULE

HEADERS += $$PWD/src/app/registration/grangeatregistration2d3d.h

SOURCES += $$PWD/src/app/registration/grangeatregistration2d3d.cpp

# NLopt library [ https://github.com/stevengj/nlopt/ ]
unix:LIBS += -L/usr/local/lib
LIBS += -lnlopt
