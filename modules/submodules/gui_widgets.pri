# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before gui_widgets.pri")

# declare module
CONFIG += GUI_WIDGETS_MODULE

QT += gui widgets

HEADERS += \
    $$PWD/../src/gui/util/qttype_utils.h \
    $$PWD/../src/gui/widgets/chunk2dview.h \
    $$PWD/../src/gui/widgets/projectionview.h \
    $$PWD/../src/gui/widgets/projectionviewer.h \
    $$PWD/../src/gui/widgets/voxelvolumeview.h \
    $$PWD/../src/gui/widgets/windowingwidget.h \
    $$PWD/../src/gui/widgets/extensionchainwidget.h

SOURCES += \
    $$PWD/../src/gui/widgets/chunk2dview.cpp \
    $$PWD/../src/gui/widgets/projectionview.cpp \
    $$PWD/../src/gui/widgets/projectionviewer.cpp \
    $$PWD/../src/gui/widgets/voxelvolumeview.cpp \
    $$PWD/../src/gui/widgets/windowingwidget.cpp \
    $$PWD/../src/gui/widgets/extensionchainwidget.cpp

FORMS += \
    $$PWD/../src/gui/widgets/projectionview.ui \
    $$PWD/../src/gui/widgets/projectionviewer.ui \
    $$PWD/../src/gui/widgets/voxelvolumeview.ui \
    $$PWD/../src/gui/widgets/windowingwidget.ui \
    $$PWD/../src/gui/widgets/extensionchainwidget.ui

