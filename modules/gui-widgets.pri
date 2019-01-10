# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_MODULE needs CTL_CORE_MODULE -> include ctl.pri before gui-widgets.pri")

# declare module
CONFIG += GUI_WIDGETS_MODULE

QT += gui 3dcore 3drender 3dextras

INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/gui/widgets/acquisitionvisualizerwidget.h \
    $$PWD/src/gui/widgets/systemvisualizerwidget.h \
    $$PWD/src/gui/widgets/voxelvolumeview.h \
    $$PWD/src/gui/widgets/windowingwidget.h \
    $$PWD/src/gui/util/qttype_utils.h \
    $$PWD/src/gui/widgets/projectionview.h

SOURCES += \
    $$PWD/src/gui/widgets/acquisitionvisualizerwidget.cpp \
    $$PWD/src/gui/widgets/systemvisualizerwidget.cpp \
    $$PWD/src/gui/widgets/voxelvolumeview.cpp \
    $$PWD/src/gui/widgets/windowingwidget.cpp \
    $$PWD/src/gui/widgets/projectionview.cpp

FORMS += \
    $$PWD/src/gui/widgets/voxelvolumeview.ui \
    $$PWD/src/gui/widgets/windowingwidget.ui \
    $$PWD/src/gui/widgets/projectionview.ui
