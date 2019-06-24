# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_MODULE needs CTL_CORE_MODULE -> include ctl.pri before gui-widgets.pri")

# declare module
CONFIG += GUI_WIDGETS_MODULE

QT += gui widgets 3dcore 3drender 3dextras

HEADERS += \
    $$PWD/src/gui/widgets/acquisitionvisualizerwidget.h \
    $$PWD/src/gui/widgets/systemvisualizerwidget.h \
    $$PWD/src/gui/widgets/voxelvolumeview.h \
    $$PWD/src/gui/widgets/windowingwidget.h \
    $$PWD/src/gui/util/qttype_utils.h \
    $$PWD/src/gui/widgets/projectionview.h \
    $$PWD/src/gui/widgets/volumeslicerwidget.h \
    $$PWD/src/gui/widgets/planevisualizer.h

SOURCES += \
    $$PWD/src/gui/widgets/acquisitionvisualizerwidget.cpp \
    $$PWD/src/gui/widgets/systemvisualizerwidget.cpp \
    $$PWD/src/gui/widgets/voxelvolumeview.cpp \
    $$PWD/src/gui/widgets/windowingwidget.cpp \
    $$PWD/src/gui/widgets/projectionview.cpp \
    $$PWD/src/gui/widgets/volumeslicerwidget.cpp \
    $$PWD/src/gui/widgets/planevisualizer.cpp


FORMS += \
    $$PWD/src/gui/widgets/voxelvolumeview.ui \
    $$PWD/src/gui/widgets/windowingwidget.ui \
    $$PWD/src/gui/widgets/projectionview.ui \
    $$PWD/src/gui/widgets/volumeslicerwidget.ui
