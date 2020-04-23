#ifndef CTL_META_CTL_OCL_H
#define CTL_META_CTL_OCL_H

/*
 * This header includes all headers of the CTL OpenCL module
 * provided that all submodules are included in the qmake project, usually by including
 * the CTL OpenCL module 'ctl_ocl.pri'.
 * Otherwise, only the headers of the added submodules will be included.
 */

#ifdef OCL_CONFIG_MODULE_AVAILABLE
#include "ocl/clfileloader.h"
#include "ocl/openclconfig.h"
#include "ocl/pinnedmem.h"
#endif // OCL_CONFIG_MODULE_AVAILABLE

#ifdef OCL_ROUTINES_MODULE_AVAILABLE
#include "processing/consistency.h"
#include "processing/imageresampler.h"
#include "processing/radontransform2d.h"
#include "processing/radontransform3d.h"
#include "processing/volumeresampler.h"
#include "processing/volumeslicer.h"
#include "projectors/raycaster.h"
#include "projectors/raycasteradapter.h"
#include "projectors/raycasterprojector.h"
#include "projectors/standardpipeline.h"
#endif // OCL_ROUTINES_MODULE_AVAILABLE

#endif // CTL_META_CTL_OCL_H
