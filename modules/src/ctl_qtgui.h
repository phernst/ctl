#ifndef CTL_META_CTL_QTGUI_H
#define CTL_META_CTL_QTGUI_H

/*
 * This header includes all headers of the CTL QtGUI module
 * provided that all submodules are included in the qmake project, usually by including
 * the CTL QtGUI module 'ctl_qtgui.pri'.
 * Otherwise, only the headers of the added submodules will be included.
 */

#ifdef GUI_WIDGETS_MODULE_AVAILABLE
#include "gui/util/qttype_utils.h"
#include "gui/widgets/chunk2dview.h"
#include "gui/widgets/extensionchainwidget.h"
#include "gui/widgets/projectionviewer.h"
#include "gui/widgets/volumeviewer.h"
#include "gui/widgets/windowingwidget.h"
#include "gui/widgets/zoomcontrolwidget.h"
#endif // GUI_WIDGETS_MODULE_AVAILABLE

#ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
#include "gui/widgets/acquisitionsetupview.h"
#include "gui/widgets/ctsystemview.h"
#include "gui/widgets/intersectionplaneview.h"
#endif // GUI_WIDGETS_3D_MODULE_AVAILABLE

#ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
#include "gui/widgets/chartviewbase.h"
#include "gui/widgets/datamodelviewer.h"
#include "gui/widgets/intervalseriesview.h"
#include "gui/widgets/lineseriesview.h"
#endif // GUI_WIDGETS_CHARTS_MODULE_AVAILABLE

#ifdef GUI_WIDGETS_OCL_AVAILABLE
#include "gui/widgets/pipelinecomposer.h"
#include "gui/widgets/volumesliceviewer.h"
#endif // GUI_WIDGETS_OCL_AVAILABLE

#endif // CTL_META_CTL_QTGUI_H
