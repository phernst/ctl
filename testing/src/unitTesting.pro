QT += testlib
QT -= gui
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS \
           QT_NO_DEBUG_OUTPUT

# MODULES
# =======
CTL_MODULE_DIR = ../../modules

# CTL_CORE_MODULE: core library
# -> dependencies: no
include($${CTL_MODULE_DIR}/ctl.pri)

# DEN_FILE_IO_MODULE: den file handling
# -> dependencies: no
include($${CTL_MODULE_DIR}/den_file_io.pri)

# NRRD_FILE_IO_MODULE: nrrd file handling
# -> dependencies: CTL_CORE_MODULE (ctl.pri)
include($${CTL_MODULE_DIR}/nrrd_file_io.pri)

# OCL_CONFIG_MODULE: OpenCLConfig
# -> dependencies: no
include($${CTL_MODULE_DIR}/ocl_config.pri)

# OCL_ROUTINES_MODULE: OpenCL based routines
# -> dependencies: CTL_CORE_MODULE (ctl.pri)
#                  OCL_CONFIG_MODULE (ocl_config.pri)
include($${CTL_MODULE_DIR}/ocl_routines.pri)

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
