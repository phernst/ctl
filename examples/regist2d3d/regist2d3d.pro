QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += main.cpp

# CTL modules
include(../../modules/ctl.pri)
include(../../modules/den_file_io.pri)
include(../../modules/nrrd_file_io.pri)
include(../../modules/ocl_config.pri)
include(../../modules/ocl_routines.pri)
include(../../modules/grangeat-2d3d-regist.pri)
