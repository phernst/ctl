# declare module
CONFIG += CTL_CORE_QTFREE_MODULE

# disable min/max macros in Windows headers
DEFINES += NOMINMAX

INCLUDEPATH += $$PWD/../src

HEADERS += \
    $$PWD/../src/mat/deg.h \
    $$PWD/../src/mat/matrix.h \
    $$PWD/../src/processing/coordinates.h

SOURCES += \
    $$PWD/../src/mat/matrix.tpp
