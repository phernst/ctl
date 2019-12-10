# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# dependency
!CTL_CORE_MODULE: error("NRRD_FILE_IO_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before nrrd_file_io.pri")

# declare module
CONFIG += NRRD_FILE_IO_MODULE

HEADERS += $$CTL_SOURCE_DIR/io/nrrd/nrrdfileio.h

SOURCES += $$CTL_SOURCE_DIR/io/nrrd/nrrdfileio.tpp
