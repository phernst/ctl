#!/bin/bash -

# Check include guard prefixes
# ============================

cp -r src src_corrected

HEADER_FILES="src_corrected/*/*.h src_corrected/*/*/*.h"

# add `CTL_` prefix to include guard macros
sed -i 's;^\(#ifndef\) \([[:upper:][:digit:]_]\+_H\);\1 CTL_\2;' $HEADER_FILES
sed -i 's;^\(#define\) \([[:upper:][:digit:]_]\+_H\);\1 CTL_\2;' $HEADER_FILES
sed -i 's;^\(#endif\ //\) \([[:upper:][:digit:]_]\+_H\);\1 CTL_\2;' $HEADER_FILES

# clean possibly introduced double "CTL_" prefix
sed -i 's;^\(#ifndef\) CTL_CTL_;\1 CTL_;' $HEADER_FILES
sed -i 's;^\(#define\) CTL_CTL_;\1 CTL_;' $HEADER_FILES
sed -i 's;^\(#endif\ //\) CTL_CTL_;\1 CTL_;' $HEADER_FILES

# compare with original HEADER_FILES
diff -r src src_corrected
if [ $? -eq 1 ]
then
	echo 'ERROR: `CTL_` prefix is missing in an include guard macro.'
	exit 1
fi

rm -r src_corrected


# Check meta header for completeness
# ==================================

# CTL main module
NB_LINES_HEADER=$(grep -c '^#include "' src/ctl.h)
NB_LINES_MODULE=$(cat submodules/ctl_core.pri submodules/den_file_io.pri submodules/nrrd_file_io.pri |
				  grep -Ec '\s+\$\$PWD/.*\.h( \\)?\s?$')

if [ $NB_LINES_HEADER -ne $NB_LINES_MODULE ]
then
	echo 'ERROR: Number of header in ctl.h / ctl.pri: ' $NB_LINES_HEADER ' / ' $NB_LINES_MODULE
	exit 1
fi

# CTL OpenCL module
NB_LINES_HEADER=$(grep -c '^#include "' src/ctl_ocl.h)
NB_LINES_MODULE=$(cat submodules/ocl_config.pri submodules/ocl_routines.pri |
				  grep -Ec '\s+\$\$PWD/.*\.h( \\)?\s?$')

if [ $NB_LINES_HEADER -ne $NB_LINES_MODULE ]
then
	echo 'ERROR: Number of header in ctl_ocl.h / ctl_ocl.pri: ' $NB_LINES_HEADER ' / ' $NB_LINES_MODULE
	exit 1
fi

# CTL QtGUI module
NB_LINES_HEADER=$(grep -c '^#include "' src/ctl_qtgui.h)
NB_LINES_MODULE=$(cat submodules/gui_widgets.pri submodules/gui_widgets_3d.pri submodules/gui_widgets_charts.pri submodules/gui_widgets_ocl.pri |
				  grep -Ec '\s+\$\$PWD/.*\.h( \\)?\s?$')

if [ $NB_LINES_HEADER -ne $NB_LINES_MODULE ]
then
	echo 'ERROR: Number of header in ctl_qtgui.h / ctl_qtgui.pri: ' $NB_LINES_HEADER ' / ' $NB_LINES_MODULE
	exit 1
fi

# CTL NLopt module
NB_LINES_HEADER=$(grep -c '^#include "' src/ctl_nlopt.h)
NB_LINES_MODULE=$(cat submodules/grangeat_2d3d_regist.pri |
				  grep -Ec '\s+\$\$PWD/.*\.h( \\)?\s?$')

if [ $NB_LINES_HEADER -ne $NB_LINES_MODULE ]
then
	echo 'ERROR: Number of header in ctl_nlopt.h / ctl_nlopt.pri: ' $NB_LINES_HEADER ' / ' $NB_LINES_MODULE
	exit 1
fi
