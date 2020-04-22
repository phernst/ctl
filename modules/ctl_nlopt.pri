# CTL NLopt module
# ================
#
# Includes CTL submodules with NLopt dependency.
#
# Install NLopt library:
#   git clone git://github.com/stevengj/nlopt
#   cd nlopt
#   mkdir build
#   cd build
#   cmake ..
#   cmake --build . --target install
#
# Note that this module depends on the CTL OpenCL module.

!GRANGEAT_2D3D_REGIST_MODULE: include(submodules/grangeat_2d3d_regist.pri)

HEADERS += $$PWD/src/ctl_nlopt.h
