# declare module
CONFIG += OCL_CONFIG_MODULE

INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/ocl/clfileloader.h \
    $$PWD/src/ocl/openclconfig.h

SOURCES += \
    $$PWD/src/ocl/clfileloader.cpp \
    $$PWD/src/ocl/openclconfig.cpp

LIBS += -lOpenCL

# copy OpenCL source files
win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/src/ocl/cl_src)\" \"$$shell_path($$DESTDIR/cl_src)\" $$escape_expand(\n\t)
!win32:QMAKE_POST_LINK += $(COPY_DIR) \"$$shell_path($$PWD/src/ocl/cl_src)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
