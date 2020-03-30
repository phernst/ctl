# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_3D_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before gui_widgets_3d.pri")
!GUI_WIDGETS_MODULE: error("GUI_WIDGETS_3D_MODULE needs GUI_WIDGETS_MODULE -> include gui_widgets.pri before gui_widgets_3d.pri")

# declare module
CONFIG += GUI_WIDGETS_3D_MODULE
DEFINES += GUI_WIDGETS_3D_MODULE_AVAILABLE

QT += 3dcore 3drender 3dextras

HEADERS += \
    $$PWD/../src/gui/widgets/acquisitionvisualizerwidget.h \
    $$PWD/../src/gui/widgets/planevisualizer.h \
    $$PWD/../src/gui/widgets/systemvisualizerwidget.h

SOURCES += \
    $$PWD/../src/gui/widgets/acquisitionvisualizerwidget.cpp \
    $$PWD/../src/gui/widgets/planevisualizer.cpp \
    $$PWD/../src/gui/widgets/systemvisualizerwidget.cpp
