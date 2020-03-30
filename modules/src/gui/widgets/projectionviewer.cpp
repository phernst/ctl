#include "projectionviewer.h"
#include "ui_projectionviewer.h"

#include <QKeyEvent>
#include <QDebug>

namespace CTL {
namespace gui {

ProjectionViewer::ProjectionViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionViewer)
{
    ui->setupUi(this);

    connect(ui->_VS_projection, &QSlider::valueChanged, this, &ProjectionViewer::showView);
    // connections for windowing
    connect(ui->_W_windowing, &WindowingWidget::windowingChanged, this, &ProjectionViewer::windowingUpdate);
    connect(ui->_W_windowing, &WindowingWidget::autoWindowingRequested, ui->_W_dataView, &Chunk2DView::setWindowingMinMax);
    connect(ui->_W_dataView, &Chunk2DView::windowingChanged, ui->_W_windowing, &WindowingWidget::setWindowDataSilent);
    // connections for zoom
    connect(ui->_W_zoomControl, &ZoomControlWidget::zoomRequested, ui->_W_dataView, &Chunk2DView::setZoom);
    connect(ui->_W_dataView, &Chunk2DView::zoomChanged, ui->_W_zoomControl, &ZoomControlWidget::setZoomValueSilent);
    // connections for live pixel info
    connect(ui->_W_dataView, &Chunk2DView::pixelInfoUnderCursor, this, &ProjectionViewer::updatePixelInfo);

    ui->_W_dataView->setLivePixelDataEnabled(true);
    ui->_W_dataView->setContrastLinePlotLabels("Position on line", "Extinction");

    resize(1000, 800);
    setWindowTitle("Projection Viewer");
}

ProjectionViewer::ProjectionViewer(ProjectionData projections, QWidget* parent)
    : ProjectionViewer(parent)
{
    setData(projections);
}

ProjectionViewer::~ProjectionViewer()
{
    delete ui;
}

void ProjectionViewer::plot(ProjectionData projections, const ModuleLayout& layout)
{
    auto viewer = new ProjectionViewer(projections);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setModuleLayout(layout);
    viewer->autoResize();
    viewer->ui->_W_dataView->setAutoMouseWindowScaling();

    viewer->show();
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

    if(_data.nbViews() > 0)
        showView(currentView()); // recompute data due to changed layout (if data is available)
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

void ProjectionViewer::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_K)
    {
        ui->_W_dataView->showContrastLinePlot();
    }
    else if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        ui->_W_dataView->saveDialog();
        event->accept();
    }

    QWidget::keyPressEvent(event);
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

} // namespace gui
} // namespace CTL
