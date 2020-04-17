# Qt specific
QT += testlib
QT -= gui
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS \
           QT_NO_DEBUG_OUTPUT

# Full optimization
CONFIG += optimize_full
unix: QMAKE_LFLAGS += -O3

# Warnings as errors
CONFIG += warn_on
win32-g++|!win32: QMAKE_CXXFLAGS += -Werror
win32-msvc*: QMAKE_CXXFLAGS += /WX

# CTL MODULES
include(../../modules/ctl.pri)
include(../../modules/ctl_ocl.pri)

# Source code of unit tests
SOURCES += \
    main.cpp \
    acquisitionsetuptest.cpp \
    ctsystemtest.cpp \
    datatypetest.cpp \
    denfileiotest.cpp \
    geometrytest.cpp \
    nrrdfileiotest.cpp \
    projectionmatrixtest.cpp \
    projectortest.cpp \
    spectrumtest.cpp

HEADERS += \
    acquisitionsetuptest.h \
    ctsystemtest.h \
    datatypetest.h \
    denfileiotest.h \
    geometrytest.h \
    nrrdfileiotest.h \
    projectionmatrixtest.h \
    projectortest.h \
    spectrumtest.h

# Copy test data
win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/../testData)\" \"$$shell_path($$DESTDIR/testData)\" $$escape_expand(\n\t)
!win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/../testData)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
