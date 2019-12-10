# Includes CTL submodules with NLopt and OpenCL dependency.
#
# Install NLopt library:
#   git clone git://github.com/stevengj/nlopt
#   cd nlopt
#   mkdir build
#   cd build
#   cmake ..
#   make
#   sudo make install
#
# Install OpenCL (platform dependent):
#   sudo apt install nvidia-opencl-dev opencl-headers

!OCL_CONFIG_MODULE:include(submodules/ocl_config.pri)
!OCL_ROUTINES_MODULE:include(submodules/ocl_routines.pri)
!GRANGEAT_2D3D_REGIST_MODULE:include(submodules/grangeat_2d3d_regist.pri)
