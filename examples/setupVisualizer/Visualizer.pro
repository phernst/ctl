QT += gui widgets 3dcore 3drender 3dinput 3dextras

TARGET = Visualizer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += mainwindow.h

FORMS += mainwindow.ui

RESOURCES += data.qrc

# CTL modules
include(../../modules/ctl.pri)
include(../../modules/gui-widgets.pri)
include(../../modules/den_file_io.pri)
