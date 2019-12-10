# optional dependency
!CTL_CORE_MODULE {
    warning("DenFileIO is not available because CTL_CORE_MODULE is missing")
    DEFINES += CTL_CORE_MODULE_NOT_AVAILABLE
    
    INCLUDEPATH += $$PWD/../src
}

# declare module
CONFIG += DEN_FILE_IO_MODULE

HEADERS += \
    $$PWD/../src/io/den/den.h \
    $$PWD/../src/io/den/den_header.h \
    $$PWD/../src/io/den/den_tpl_deduct.h \
    $$PWD/../src/io/den/den_utils.h \
    $$PWD/../src/io/den/dfileformat.h \
    $$PWD/../src/io/den/denfileio.h

SOURCES += \
    $$PWD/../src/io/den/den_header.tpp \
    $$PWD/../src/io/den/den_utils.tpp \
    $$PWD/../src/io/den/dfileformat.tpp
