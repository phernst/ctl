# Includes GUI-related CTL submodules that depends on the following Qt modules:
# gui widgets 3dcore 3drender 3dextras.
# If the CTL OpenCL module has been included, this also includes the CTL submodule(s) for OpenCL-dependent widgets.
#
# Install Qt 3D:
#   apt install qt3d5-dev

!GUI_WIDGETS_MODULE:    include(submodules/gui_widgets.pri)
!GUI_WIDGETS_3D_MODULE: include(submodules/gui_widgets_3d.pri)

# if OpenCL is available, also include submodule(s) with OpenCL-dependent widgets
OCL_ROUTINES_MODULE {
    !GUI_WIDGETS_OCL: include(submodules/gui_widgets_ocl.pri)
} else {
    message("No OpenCL-dependent submodules will be included. Include the OpenCL module in advance to change this behavior.")
}
