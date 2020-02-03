QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += main.cpp

# CTL modules
include(../../modules/ctl.pri)
include(../../modules/ctl_ocl.pri)
