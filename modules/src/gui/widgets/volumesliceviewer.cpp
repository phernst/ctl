#include "volumesliceviewer.h"
#include "ui_volumesliceviewer.h"
#include "img/compositevolume.h"
#ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
#include "gui/widgets/intersectionplaneview.h"
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

VolumeSliceViewer::VolumeSliceViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumeSliceViewer)
{
    ui->setupUi(this);

    VoxelVolume<float> volume(0, 0, 0);

#ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
    _3dViewer = new IntersectionPlaneView;
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
    connect(ui->_W_windowing, &WindowingWidget::windowingChanged, this, &VolumeSliceViewer::windowingUpdate);
    connect(ui->_W_windowing, &WindowingWidget::autoWindowingRequested, ui->_w_sliceView, &Chunk2DView::setWindowingMinMax);
    connect(ui->_w_sliceView, &Chunk2DView::windowingChanged, ui->_W_windowing, &WindowingWidget::setWindowDataSilent);
    // connections for zoom
    connect(ui->_W_zoomControl, &ZoomControlWidget::zoomRequested, ui->_w_sliceView, &Chunk2DView::setZoom);
    connect(ui->_w_sliceView, &Chunk2DView::zoomChanged, ui->_W_zoomControl, &ZoomControlWidget::setZoomValueSilent);
    // connections for live pixel info
    connect(ui->_w_sliceView, &Chunk2DView::pixelInfoUnderCursor, this, &VolumeSliceViewer::updatePixelInfo);
    ui->_w_sliceView->setLivePixelDataEnabled(true);
    ui->_w_sliceView->setContrastLinePlotLabels("Position on line", "Attenuation");

    connect(ui->_SB_azimuth, SIGNAL(valueChanged(double)), SLOT(planeChange()));
    connect(ui->_SB_polar, SIGNAL(valueChanged(double)), SLOT(planeChange()));
    connect(ui->_SB_distance, SIGNAL(valueChanged(double)), SLOT(planeChange()));

    resize(1400, 800);
    setWindowTitle("Volume slice viewer");
}

VolumeSliceViewer::~VolumeSliceViewer()
{
    delete ui;
}

void VolumeSliceViewer::setData(const VoxelVolume<float>& volume)
{
    _slicer.reset(new OCL::VolumeSlicer(volume));

    dataChange();
}

void VolumeSliceViewer::plot(const VoxelVolume<float>& volume)
{
    auto viewer = new VolumeSliceViewer;
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setData(volume);

    viewer->show();
}

void VolumeSliceViewer::dataChange()
{
    #ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
    _3dViewer->setVolumeDim(_slicer->volDim(), _slicer->volVoxSize(), _slicer->volOffset());
    _3dViewer->setPlaneSize( { _slicer->sliceDimensions().width * _slicer->sliceResolution(),
                             _slicer->sliceDimensions().height * _slicer->sliceResolution() } );
    #endif // GUI_WIDGETS_3D_MODULE_AVAILABLE

    recomputeSlice();
}

void VolumeSliceViewer::planeChange()
{
    #ifdef GUI_WIDGETS_3D_MODULE_AVAILABLE
    _3dViewer->setPlaneParameter(qDegreesToRadians(ui->_SB_azimuth->value()),
                                 qDegreesToRadians(ui->_SB_polar->value()),
                                 ui->_SB_distance->value());
    #endif // GUI_WIDGETS_3D_MODULE_AVAILABLE

    recomputeSlice();
}

void VolumeSliceViewer::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_K)
    {
        ui->_w_sliceView->showContrastLinePlot();
    }
    else if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        ui->_w_sliceView->saveDialog();
        event->accept();
    }

    QWidget::keyPressEvent(event);
}

void VolumeSliceViewer::recomputeSlice()
{
    auto slice = _slicer->slice(qDegreesToRadians(ui->_SB_azimuth->value()),
                                qDegreesToRadians(ui->_SB_polar->value()),
                                ui->_SB_distance->value());

    ui->_w_sliceView->setData(slice);
}

void VolumeSliceViewer::updatePixelInfo(int x, int y, float value)
{
    QString info = QStringLiteral("(") + QString::number(x) + QStringLiteral(" , ")
                                       + QString::number(y) + QStringLiteral("): ")
                                       + QString::number(value);
    ui->_L_pixelInfo->setText(info);
}

void VolumeSliceViewer::windowingUpdate()
{
    auto newWindowing = ui->_W_windowing->windowFromTo();
    ui->_w_sliceView->setWindowing(newWindowing.first, newWindowing.second);
}

} // namespace gui
} // namespace CTL

