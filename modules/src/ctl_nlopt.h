#ifndef CTL_META_CTL_NLOPT_H
#define CTL_META_CTL_NLOPT_H

/*
 * This header includes all headers of the CTL NLopt module
 * provided that all submodules are included in the qmake project, usually by including
 * the CTL NLopt module 'ctl_nlopt.pri'.
 * Otherwise, only the headers of the added submodules will be included.
 */

#ifdef GRANGEAT_2D3D_REGIST_MODULE_AVAILABLE
#include "app/registration/grangeatregistration2d3d.h"
#endif // GRANGEAT_2D3D_REGIST_MODULE_AVAILABLE

#endif // CTL_META_CTL_NLOPT_H
