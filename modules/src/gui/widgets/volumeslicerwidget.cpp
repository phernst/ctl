#include "volumeslicerwidget.h"
#include "ui_volumeslicerwidget.h"
#include "img/compositevolume.h"
#ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
#include "gui/widgets/planevisualizer.h"
#endif
#ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
#include "gui/widgets/lineseriesview.h"
#else
#include <QMessageBox>
#endif

#include <QKeyEvent>
#include <qmath.h>

namespace CTL {
namespace gui {

VolumeSlicerWidget::VolumeSlicerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumeSlicerWidget)
{
    ui->setupUi(this);

    CTL::VoxelVolume<float> volume(0, 0, 0);

#ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
    _3dViewer = new PlaneVisualizer;
    ui->_w_3dViewer->addWidget(_3dViewer);
    ui->_w_3dViewer->setCurrentWidget(_3dViewer);
    _3dViewer->setPlaneSize( { 0.0 , 0.0 } );
    _3dViewer->setVolumeDim(volume);

    connect(ui->_PB_resetCamera, SIGNAL(clicked()), _3dViewer, SLOT(resetCamera()));
#else // no 3D viewer (for plane visualization) available
    auto label = new QLabel("3D Viewer not available. \n"
                            "(Requires 'gui_widgets_3d.pri' submodule.)");
    label->setAlignment(Qt::AlignCenter);
    ui->_w_3dViewer->addWidget(label);
    ui->_w_3dViewer->setCurrentWidget(label);
    ui->_w_3dViewer->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    ui->_PB_resetCamera->setEnabled(false);
#endif

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

    resize(1400, 800);
    setWindowTitle("Volume Slicer");
}

VolumeSlicerWidget::~VolumeSlicerWidget()
{
    delete ui;
}

void VolumeSlicerWidget::setData(const CTL::VoxelVolume<float>& volume)
{
    _slicer.reset(new CTL::OCL::VolumeSlicer(volume));

    dataChange();
}

void VolumeSlicerWidget::dataChange()
{
    #ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
    _3dViewer->setVolumeDim(_slicer->volDim(), _slicer->volOffset(), _slicer->volVoxSize());
    _3dViewer->setPlaneSize( { _slicer->sliceDimensions().width * _slicer->sliceResolution(),
                             _slicer->sliceDimensions().height * _slicer->sliceResolution() } );
    #endif // GUI_WIDGETS_3D_MODULE_AVAILABLE

    recomputeSlice();
}

void VolumeSlicerWidget::planeChange()
{
    #ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
    _3dViewer->setPlaneParameter(qDegreesToRadians(ui->_SB_azimuth->value()),
                                 qDegreesToRadians(ui->_SB_polar->value()),
                                 ui->_SB_distance->value());
    #endif // GUI_WIDGETS_3D_MODULE_AVAILABLE

    recomputeSlice();
}

void VolumeSlicerWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_K)
    {
    #ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
        LineSeriesView::plot(ui->_w_sliceView->contrastLine(), "Distance on line", "Attenuation");
        event->accept();
    #else
        QMessageBox::information(this, "Contrast line plot", "Contrast line plot not available.\n"
                                                             "(Requires 'gui_widgets_charts.pri' submodule.)");
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

} // namespace gui
} // namespace CTL

