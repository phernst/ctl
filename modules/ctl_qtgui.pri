# Includes GUI-related CTL submodules that depends on the following Qt modules:
# gui widgets 3dcore 3drender 3dextras charts.
# If the CTL OpenCL module has been included, this also includes the CTL submodule(s) for OpenCL-dependent widgets.
#
# Install Qt3D:
#   apt install qt3d5-dev
# Install QtCharts:
#   apt install libqt5charts5-dev

!GUI_WIDGETS_MODULE:        include(submodules/gui_widgets.pri)
!GUI_WIDGETS_3D_MODULE:     include(submodules/gui_widgets_3d.pri)
!GUI_WIDGETS_CHARTS_MODULE: include(submodules/gui_widgets_charts.pri)

# if OpenCL is available, also include submodule(s) with OpenCL-dependent widgets
OCL_ROUTINES_MODULE {
    !GUI_WIDGETS_OCL: include(submodules/gui_widgets_ocl.pri)
} else {
    message("No OpenCL-dependent submodules will be included. Include the OpenCL module in advance to change this behavior.")
}
