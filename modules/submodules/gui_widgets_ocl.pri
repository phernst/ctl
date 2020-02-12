# dependency
!CTL_CORE_MODULE: error("GUI_WIDGETS_3D_MODULE needs CTL_CORE_MODULE -> include ctl_core.pri before gui_widgets_3d.pri")
!GUI_WIDGETS_MODULE: error("GUI_WIDGETS_3D_MODULE needs GUI_WIDGETS_MODULE -> include gui_widgets.pri before gui_widgets_3d.pri")
!OCL_CONFIG_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs OCL_CONFIG_MODULE -> include ocl_config.pri before grangeat_2d3d_regist.pri")
!OCL_ROUTINES_MODULE: error("GRANGEAT_2D3D_REGIST_MODULE needs OCL_ROUTINES_MODULE -> include ocl_routines.pri before grangeat_2d3d_regist.pri")

# declare module
CONFIG += GUI_WIDGETS_OCL

HEADERS += \
    $$PWD/../src/gui/widgets/pipelinecomposerwidget.h

SOURCES += \
    $$PWD/../src/gui/widgets/pipelinecomposerwidget.cpp

FORMS += \
    $$PWD/../src/gui/widgets/pipelinecomposerwidget.ui
