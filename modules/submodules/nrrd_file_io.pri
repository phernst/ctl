# dependency
!CTL_CORE_MODULE: error("NRRD_FILE_IO_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before nrrd_file_io.pri")

# declare module
CONFIG += NRRD_FILE_IO_MODULE

HEADERS += $$PWD/../src/io/nrrd/nrrdfileio.h

SOURCES += $$PWD/../src/io/nrrd/nrrdfileio.tpp
