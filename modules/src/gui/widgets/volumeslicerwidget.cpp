#include "volumeslicerwidget.h"
#include "ui_volumeslicerwidget.h"
#include "img/compositevolume.h"
#include <qmath.h>

VolumeSlicerWidget::VolumeSlicerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumeSlicerWidget)
{
    ui->setupUi(this);

    CTL::VoxelVolume<float> volume(0, 0, 0);
    ui->_w_3dViewer->setPlaneSize( { 0.0 , 0.0 } );
    ui->_w_3dViewer->setVolumeDim(volume);

#ifdef OCL_CONFIG_MODULE_AVAILABLE
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

void VolumeSlicerWidget::recomputeSlice()
{
    auto slice = _slicer->slice(qDegreesToRadians(ui->_SB_azimuth->value()),
                                qDegreesToRadians(ui->_SB_polar->value()),
                                ui->_SB_distance->value());

    ui->_w_sliceView->setData(CTL::ProjectionData(CTL::SingleViewData{slice}));
}
#endif
