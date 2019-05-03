QT += testlib

QT -= gui

CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS \
           QT_NO_DEBUG_OUTPUT

SOURCES += \
    main.cpp \
    nrrdfileiotest.cpp \
    projectionmatrixtest.cpp \
    ctsystemtest.cpp \
    datatypetest.cpp \
    geometrytest.cpp \
    denfileiotest.cpp \
    projectortest.cpp \
    spectrumtest.cpp \
    acquisitionsetuptest.cpp

HEADERS += \
    nrrdfileiotest.h \
    projectionmatrixtest.h \
    ctsystemtest.h \
    datatypetest.h \
    geometrytest.h \
    denfileiotest.h \
    projectortest.h \
    spectrumtest.h \
    acquisitionsetuptest.h

# MODULES
# =======

# CTL_CORE_MODULE: core library
# -> dependencies: no
include(../../modules/ctl.pri)

# DEN_FILE_IO_MODULE: den file handling
# -> dependencies: no
include(../../modules/den_file_io.pri)

# NRRD_FILE_IO_MODULE: nrrd file handling
# -> dependencies: no
include(../../modules/nrrd_file_io.pri)

# OCL_CONFIG_MODULE: OpenCLConfig
# -> dependencies: no
include(../../modules/ocl_config.pri)

# OCL_PROJECTORS_MODULE: OpenCL based projectors
# -> dependencies: CTL_CORE_MODULE (ctl.pri)
#                  OCL_CONFIG_MODULE (ocl_config.pri)
include(../../modules/ocl_projectors.pri)

win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/../testData)\" \"$$shell_path($$DESTDIR/testData)\" $$escape_expand(\n\t)
!win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/../testData)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
