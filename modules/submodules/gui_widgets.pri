!GUI_WIDGETS_MODULE {

# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before gui_widgets.pri")

# declare module
CONFIG += GUI_WIDGETS_MODULE

QT += gui widgets

HEADERS += \
    $$CTL_SOURCE_DIR/gui/util/qttype_utils.h \
    $$CTL_SOURCE_DIR/gui/widgets/projectionview.h \
    $$CTL_SOURCE_DIR/gui/widgets/voxelvolumeview.h \
    $$CTL_SOURCE_DIR/gui/widgets/windowingwidget.h

SOURCES += \
    $$CTL_SOURCE_DIR/gui/widgets/projectionview.cpp \
    $$CTL_SOURCE_DIR/gui/widgets/voxelvolumeview.cpp \
    $$CTL_SOURCE_DIR/gui/widgets/windowingwidget.cpp

FORMS += \
    $$CTL_SOURCE_DIR/gui/widgets/projectionview.ui \
    $$CTL_SOURCE_DIR/gui/widgets/voxelvolumeview.ui \
    $$CTL_SOURCE_DIR/gui/widgets/windowingwidget.ui

} # GUI_WIDGETS_MODULE
