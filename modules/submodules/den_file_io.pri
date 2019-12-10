# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# optional dependency
!CTL_CORE_MODULE {
    warning("DenFileIO is not available because CTL_CORE_MODULE is missing")
    DEFINES += CTL_CORE_MODULE_NOT_AVAILABLE
    
    INCLUDEPATH += $$CTL_SOURCE_DIR
}

# declare module
CONFIG += DEN_FILE_IO_MODULE

HEADERS += \
    $$CTL_SOURCE_DIR/io/den/den.h \
    $$CTL_SOURCE_DIR/io/den/den_header.h \
    $$CTL_SOURCE_DIR/io/den/den_tpl_deduct.h \
    $$CTL_SOURCE_DIR/io/den/den_utils.h \
    $$CTL_SOURCE_DIR/io/den/dfileformat.h \
    $$CTL_SOURCE_DIR/io/den/denfileio.h

SOURCES += \
    $$CTL_SOURCE_DIR/io/den/den_header.tpp \
    $$CTL_SOURCE_DIR/io/den/den_utils.tpp \
    $$CTL_SOURCE_DIR/io/den/dfileformat.tpp
