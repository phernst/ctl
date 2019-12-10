# Includes all CTL submodules that have only OpenCL dependency
#
# Install OpenCL (platform dependent):
#   sudo apt install nvidia-opencl-dev opencl-headers

!OCL_CONFIG_MODULE:include(submodules/ocl_config.pri)
!OCL_ROUTINES_MODULE:include(submodules/ocl_routines.pri)
