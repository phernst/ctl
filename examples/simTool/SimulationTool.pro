QT += core gui widgets

TARGET = SimulationTool
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS \
           QT_NO_DEBUG_OUTPUT

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

# CTL modules
include(../../modules/ctl.pri)
include(../../modules/gui-widgets.pri)
include(../../modules/den_file_io.pri)
include(../../modules/ocl_config.pri)
include(../../modules/ocl_routines.pri)
