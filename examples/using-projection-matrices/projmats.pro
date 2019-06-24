QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += main.cpp

include(../../modules/ctl.pri)
include(../../modules/den_file_io.pri)
include(../../modules/ocl_config.pri)
include(../../modules/ocl_routines.pri)
