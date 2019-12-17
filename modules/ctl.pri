# Includes all CTL submodules that have no further dependencies except Qt
#
# Install Qt libraries:
#   sudo apt install qt5-default

!CTL_CORE_MODULE:     include(submodules/ctl_core.pri)
!DEN_FILE_IO_MODULE:  include(submodules/den_file_io.pri)
!NRRD_FILE_IO_MODULE: include(submodules/nrrd_file_io.pri)
