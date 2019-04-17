# dependency
!CTL_CORE_MODULE: error("NRRD_FILE_IO_MODULE needs CTL_CORE_MODULE -> include ctl.pri before nrrd_file_io.pri")

# declare module
CONFIG += NRRD_FILE_IO_MODULE

INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/io/nrrd/nrrdfileio.h \
    $$PWD/src/io/nrrd/nrrdfileio.tpp


SOURCES += \

