#include "volumeslicerwidget.h"
#include "ui_volumeslicerwidget.h"
#include "img/compositevolume.h"
#include <qmath.h>

namespace CTL {
namespace gui {

VolumeSlicerWidget::VolumeSlicerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumeSlicerWidget)
{
    ui->setupUi(this);

    CTL::VoxelVolume<float> volume(0, 0, 0);
    ui->_w_3dViewer->setPlaneSize( { 0.0 , 0.0 } );
    ui->_w_3dViewer->setVolumeDim(volume);

#ifdef OCL_CONFIG_MODULE_AVAILABLE

    // connections for windowing
    connect(ui->_W_windowing, &WindowingWidget::windowingChanged, this, &VolumeSlicerWidget::windowingUpdate);
    connect(ui->_W_windowing, &WindowingWidget::autoWindowingRequested, ui->_w_sliceView, &Chunk2DView::setWindowingMinMax);
    connect(ui->_w_sliceView, &Chunk2DView::windowingChanged, ui->_W_windowing, &WindowingWidget::setWindowDataSilent);
    // connections for zoom
    connect(ui->_W_zoomControl, &ZoomControlWidget::zoomRequested, ui->_w_sliceView, &Chunk2DView::setZoom);
    connect(ui->_w_sliceView, &Chunk2DView::zoomChanged, ui->_W_zoomControl, &ZoomControlWidget::setZoomValueSilent);
    // connections for live pixel info
    connect(ui->_w_sliceView, &Chunk2DView::pixelInfoUnderCursor, this, &VolumeSlicerWidget::updatePixelInfo);

    ui->_w_sliceView->setLivePixelDataEnabled(true);

    connect(ui->_SB_azimuth, SIGNAL(valueChanged(double)), SLOT(planeChange()));
    connect(ui->_SB_polar, SIGNAL(valueChanged(double)), SLOT(planeChange()));
    connect(ui->_SB_distance, SIGNAL(valueChanged(double)), SLOT(planeChange()));
    connect(ui->_PB_resetCamera, SIGNAL(clicked()), ui->_w_3dViewer, SLOT(resetCamera()));

#endif

#ifndef OCL_CONFIG_MODULE_AVAILABLE
    qWarning() << "OCL_CONFIG_MODULE is required!"
                  "VolumeSlicerWidget needs OpenCL to compute sliced images."
                  "No functionality will be available";
#endif

    resize(1200, 800);
    setWindowTitle("Volume Slicer");
}

VolumeSlicerWidget::~VolumeSlicerWidget()
{
    delete ui;
}

#ifdef OCL_CONFIG_MODULE_AVAILABLE
void VolumeSlicerWidget::setData(const CTL::VoxelVolume<float>& volume)
{
    _slicer.reset(new CTL::OCL::VolumeSlicer(volume));

    dataChange();
}

void VolumeSlicerWidget::dataChange()
{
    ui->_w_3dViewer->setVolumeDim(_slicer->volDim(), _slicer->volOffset(), _slicer->volVoxSize());
    ui->_w_3dViewer->setPlaneSize( { _slicer->sliceDimensions().width * _slicer->sliceResolution(),
                                     _slicer->sliceDimensions().height * _slicer->sliceResolution() } );

    recomputeSlice();
}

void VolumeSlicerWidget::planeChange()
{
    ui->_w_3dViewer->setPlaneParameter(qDegreesToRadians(ui->_SB_azimuth->value()),
                                       qDegreesToRadians(ui->_SB_polar->value()),
                                       ui->_SB_distance->value());

    recomputeSlice();
}

void VolumeSlicerWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_K)
    {
    #ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
        LineSeriesView::plot(ui->_w_sliceView->contrastLine(), "Distance on line", "Attenuation");
        event->accept();
    #endif
    }
    else if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        ui->_w_sliceView->saveDialog();
        event->accept();
    }

    QWidget::keyPressEvent(event);
}

void VolumeSlicerWidget::recomputeSlice()
{
    auto slice = _slicer->slice(qDegreesToRadians(ui->_SB_azimuth->value()),
                                qDegreesToRadians(ui->_SB_polar->value()),
                                ui->_SB_distance->value());

    ui->_w_sliceView->setData(slice);
}

void VolumeSlicerWidget::updatePixelInfo(int x, int y, float value)
{
    QString info = QStringLiteral("(") + QString::number(x) + QStringLiteral(" , ")
                                       + QString::number(y) + QStringLiteral("): ")
                                       + QString::number(value);
    ui->_L_pixelInfo->setText(info);
}

void VolumeSlicerWidget::windowingUpdate()
{
    auto newWindowing = ui->_W_windowing->windowFromTo();
    ui->_w_sliceView->setWindowing(newWindowing.first, newWindowing.second);
}

#endif // OCL_CONFIG_MODULE_AVAILABLE

} // namespace gui
} // namespace CTL

