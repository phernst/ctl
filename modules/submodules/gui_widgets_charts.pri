# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_CHARTS_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before gui_widgets_charts.pri")
!GUI_WIDGETS_MODULE: error("GUI_WIDGETS_CHARTS_MODULE needs GUI_WIDGETS_MODULE -> include gui_widgets.pri before gui_widgets_charts.pri")

# declare module
CONFIG += GUI_WIDGETS_CHARTS_MODULE
DEFINES += GUI_WIDGETS_CHARTS_MODULE_AVAILABLE

QT += charts

HEADERS += \
    $$PWD/../src/gui/widgets/intervalseriesview.h \
    $$PWD/../src/gui/widgets/lineseriesview.h


SOURCES += \
    $$PWD/../src/gui/widgets/intervalseriesview.cpp \
    $$PWD/../src/gui/widgets/lineseriesview.cpp


FORMS += \


