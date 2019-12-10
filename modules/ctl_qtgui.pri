# Includes GUI-related CTL submodules that depends on the following Qt modules:
# gui widgets 3dcore 3drender 3dextras.
#
# Install Qt 3D:
#   apt install qt3d5-dev

!GUI_WIDGETS_MODULE:include(submodules/gui_widgets.pri)
!GUI_WIDGETS_3D_MODULE:include(submodules/gui_widgets_3d.pri)
