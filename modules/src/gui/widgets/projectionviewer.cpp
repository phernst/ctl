#include "projectionviewer.h"
#include "ui_projectionviewer.h"

namespace CTL {
namespace gui {

ProjectionViewer::ProjectionViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionViewer)
{
    ui->setupUi(this);

    connect(ui->_VS_projection, &QSlider::valueChanged, this, &ProjectionViewer::showView);
    connect(ui->_W_windowing, &WindowingWidget::windowingChanged, this, &ProjectionViewer::windowingUpdate);
    connect(ui->_W_windowing, &WindowingWidget::autoWindowingRequested, ui->_W_dataView, &Chunk2DView::setWindowingMinMax);
    connect(ui->_W_dataView, &Chunk2DView::zoomChanged, this, &ProjectionViewer::setZoomValueSilent);
    connect(ui->_W_dataView, SIGNAL(windowingChanged(double, double)), ui->_W_windowing, SLOT(setWindowDataSilent(double, double)));
    connect(ui->_SB_zoom, SIGNAL(valueChanged(double)), ui->_W_dataView, SLOT(setZoom(double)));
}

ProjectionViewer::~ProjectionViewer()
{
    delete ui;
}

void ProjectionViewer::setData(ProjectionData projections)
{
    _data = std::move(projections);

    updateSliderRange();

    if(_data.nbViews())
        showView(0);
}

void ProjectionViewer::setModuleLayout(const CTL::ModuleLayout &layout)
{
    _modLayout = layout;
    showView(currentView()); // recompute data due to changed layout
}

int ProjectionViewer::currentView() const
{
    return ui->_VS_projection->value();
}

void ProjectionViewer::showView(int view)
{
    ui->_L_view->setText(QString::number(view));
    ui->_W_dataView->setData(_data.view(view).combined(_modLayout));
}

void ProjectionViewer::updateSliderRange()
{
    ui->_VS_projection->setMaximum(_data.dimensions().nbViews- 1);
}

void ProjectionViewer::windowingUpdate()
{
    auto newWindowing = ui->_W_windowing->windowFromTo();
    ui->_W_dataView->setWindowing(newWindowing.first, newWindowing.second);
}

void ProjectionViewer::setZoomValueSilent(double zoom)
{
    ui->_SB_zoom->blockSignals(true);
    ui->_SB_zoom->setValue(zoom);
    ui->_SB_zoom->blockSignals(false);
}

} // namespace gui
} // namespace CTL
