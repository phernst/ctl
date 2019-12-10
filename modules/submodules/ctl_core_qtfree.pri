# define source directory
CTL_SOURCE_DIR = $$PWD/../src

# declare module
CONFIG += CTL_CORE_QTFREE_MODULE

# disable min/max macros in Windows headers
DEFINES += NOMINMAX

INCLUDEPATH += $$CTL_SOURCE_DIR

HEADERS += \
    $$CTL_SOURCE_DIR/mat/deg.h \
    $$CTL_SOURCE_DIR/mat/matrix.h \
    $$CTL_SOURCE_DIR/processing/coordinates.h

SOURCES += \
    $$CTL_SOURCE_DIR/mat/matrix.tpp
