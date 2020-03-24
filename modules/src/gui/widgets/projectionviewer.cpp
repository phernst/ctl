#include "projectionviewer.h"
#include "ui_projectionviewer.h"

#include <QDebug>

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
    connect(ui->_W_dataView, &Chunk2DView::windowingChanged, ui->_W_windowing, &WindowingWidget::setWindowDataSilent);
    connect(ui->_SB_zoom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui->_W_dataView, &Chunk2DView::setZoom);
    connect(ui->_W_dataView, &Chunk2DView::pixelInfoUnderCursor, this, &ProjectionViewer::updatePixelInfo);

    resize(1000, 800);
    setWindowTitle("Projection Viewer");
}

ProjectionViewer::ProjectionViewer(ProjectionData projections, QWidget* parent)
    : ProjectionViewer(parent)
{
    setData(projections);

    ui->_W_dataView->setLivePixelDataEnabled(true);

    autoResize();
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

void ProjectionViewer::setModuleLayout(const ModuleLayout &layout)
{
    _modLayout = layout;
    showView(currentView()); // recompute data due to changed layout
}

int ProjectionViewer::currentView() const
{
    return ui->_VS_projection->value();
}

void ProjectionViewer::autoResize()
{
    static const auto margin = QSize(110, 110);
    ui->_W_dataView->autoResize();
    resize(ui->_W_dataView->size() + margin);
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

void ProjectionViewer::updatePixelInfo(int x, int y, float value)
{
    ui->_L_pixelInfo->setText("(" + QString::number(x) + " , " + QString::number(y) + "): "
                              + QString::number(value));
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
