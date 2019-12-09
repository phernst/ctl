!GUI_WIDGETS_3D_MODULE {

# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_3D_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before gui_widgets_3d.pri")
!GUI_WIDGETS_MODULE: error("GUI_WIDGETS_3D_MODULE needs GUI_WIDGETS_MODULE -> include gui_widgets.pri before gui_widgets_3d.pri")

# declare module
CONFIG += GUI_WIDGETS_3D_MODULE

QT += 3dcore 3drender 3dextras

HEADERS += \
    $$CTL_SOURCE_DIR/gui/widgets/acquisitionvisualizerwidget.h \
    $$CTL_SOURCE_DIR/gui/widgets/planevisualizer.h \
    $$CTL_SOURCE_DIR/gui/widgets/volumeslicerwidget.h \
    $$CTL_SOURCE_DIR/gui/widgets/systemvisualizerwidget.h

SOURCES += \
    $$CTL_SOURCE_DIR/gui/widgets/acquisitionvisualizerwidget.cpp \
    $$CTL_SOURCE_DIR/gui/widgets/planevisualizer.cpp \
    $$CTL_SOURCE_DIR/gui/widgets/volumeslicerwidget.cpp \
    $$CTL_SOURCE_DIR/gui/widgets/systemvisualizerwidget.cpp

FORMS += \
    $$CTL_SOURCE_DIR/gui/widgets/volumeslicerwidget.ui

} # GUI_WIDGETS_3D_MODULE
