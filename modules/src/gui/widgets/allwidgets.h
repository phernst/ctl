#ifndef CTL_ALLWIDGETS_H
#define CTL_ALLWIDGETS_H

#include "gui/widgets/datamodelviewer.h"
#include "gui/widgets/pipelinecomposer.h"
#include "gui/widgets/projectionviewer.h"
#include "gui/widgets/volumesliceviewer.h"
#include "gui/widgets/volumeviewer.h"

#include "gui/widgets/acquisitionsetupview.h"
#include "gui/widgets/ctsystemview.h"
#include "gui/widgets/chunk2dview.h"
#include "gui/widgets/intervalseriesview.h"
#include "gui/widgets/lineseriesview.h"
#include "gui/widgets/intersectionplaneview.h"

#include "gui/widgets/extensionchainwidget.h"
#include "gui/widgets/windowingwidget.h"
#include "gui/widgets/zoomcontrolwidget.h"

namespace CTL {
namespace gui {
namespace assist {

inline void plot(AcquisitionSetup setup, uint maxNbViews = 100, bool sourceOnly = false,
                 float visualScale = 50.0f)
{ AcquisitionSetupView::plot(std::move(setup), maxNbViews, sourceOnly, visualScale); }

inline void plot(Chunk2D<float> data,
                 QPair<double,double> windowing = qMakePair(0.0, 0.0), double zoom = 1.0)
{ Chunk2DView::plot(std::move(data), windowing, zoom); }

inline void plot(CompositeVolume data)
{ VolumeViewer::plot(std::move(data)); }

inline void plot(ProjectionData projections, const ModuleLayout& layout = {})
{ ProjectionViewer::plot(std::move(projections), layout); }

inline void plot(SimpleCTsystem system, float visualScale = 50.0f)
{ CTSystemView::plot(std::move(system), visualScale); }

inline void plot(SpectralVolumeData data)
{ VolumeViewer::plot(std::move(data)); }

inline void plot(std::shared_ptr<AbstractDataModel> model,
                 const QString& labelX = "x", const QString& labelY = "y")
{ DataModelViewer::plot(std::move(model),labelX, labelY); }

inline void plot(const IntervalDataSeries& intervalSeries, const QString& labelX = "x",
                 const QString& labelY = "y", bool logAxisY = false)
{ IntervalSeriesView::plot(intervalSeries, labelX, labelY, logAxisY); }

inline void plot(const XYDataSeries& lineSeries, const QString& labelX = "x",
                 const QString& labelY = "y", bool logAxisY = false)
{ LineSeriesView::plot(lineSeries, labelX, labelY, logAxisY); }

}
}
}
#endif // CTL_ALLWIDGETS_H
