# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_3D_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before gui_widgets_3d.pri")
!GUI_WIDGETS_MODULE: error("GUI_WIDGETS_3D_MODULE needs GUI_WIDGETS_MODULE -> include gui_widgets.pri before gui_widgets_3d.pri")

# declare module
CONFIG += GUI_WIDGETS_3D_MODULE
DEFINES += GUI_WIDGETS_3D_MODULE_AVAILABLE

QT += 3dcore 3drender 3dextras

HEADERS += \
    $$PWD/../src/gui/widgets/acquisitionsetupview.h \
    $$PWD/../src/gui/widgets/ctsystemview.h \
    $$PWD/../src/gui/widgets/intersectionplaneview.h

SOURCES += \
    $$PWD/../src/gui/widgets/acquisitionsetupview.cpp \
    $$PWD/../src/gui/widgets/ctsystemview.cpp \
    $$PWD/../src/gui/widgets/intersectionplaneview.cpp
