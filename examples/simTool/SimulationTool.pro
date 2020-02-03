QT += core gui widgets

TARGET = SimulationTool
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS \
           QT_NO_DEBUG_OUTPUT

CONFIG += c++11

SOURCES += main.cpp \
           mainwindow.cpp

HEADERS += mainwindow.h

FORMS += mainwindow.ui

# CTL modules
include(../../modules/ctl.pri)
include(../../modules/ctl_ocl.pri)
include(../../modules/ctl_qtgui.pri)
