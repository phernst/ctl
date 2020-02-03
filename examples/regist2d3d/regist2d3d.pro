QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

SOURCES += main.cpp

# CTL modules
include(../../modules/ctl.pri)
include(../../modules/ctl_nlopt.pri)
