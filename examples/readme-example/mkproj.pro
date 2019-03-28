QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += main.cpp

include(../../modules/ctl.pri)
include(../../modules/den_file_io.pri)
include(../../modules/ocl_config.pri)
include(../../modules/ocl_projectors.pri)
