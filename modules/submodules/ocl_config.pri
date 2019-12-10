# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# completely independent module
!CTL_CORE_MODULE: INCLUDEPATH += $$CTL_SOURCE_DIR

# declare module
CONFIG += OCL_CONFIG_MODULE
DEFINES += OCL_CONFIG_MODULE_AVAILABLE

HEADERS += \
    $$CTL_SOURCE_DIR/ocl/clfileloader.h \
    $$CTL_SOURCE_DIR/ocl/openclconfig.h \
    $$CTL_SOURCE_DIR/ocl/pinnedmem.h

SOURCES += \
    $$CTL_SOURCE_DIR/ocl/clfileloader.cpp \
    $$CTL_SOURCE_DIR/ocl/openclconfig.cpp \
    $$CTL_SOURCE_DIR/ocl/pinnedmem.cpp

LIBS += -lOpenCL

# copy OpenCL source files
win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$CTL_SOURCE_DIR/ocl/cl_src)\" \"$$shell_path($$DESTDIR/cl_src)\" $$escape_expand(\n\t)
!win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$CTL_SOURCE_DIR/ocl/cl_src)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
