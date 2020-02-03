QT += testlib
QT -= gui
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS \
           QT_NO_DEBUG_OUTPUT

# MODULES
# =======
include(../../modules/ctl.pri)
include(../../modules/ctl_ocl.pri)


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

win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/../testData)\" \"$$shell_path($$DESTDIR/testData)\" $$escape_expand(\n\t)
!win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/../testData)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
