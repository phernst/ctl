#ifndef VOLUMESLICERWIDGET_H
#define VOLUMESLICERWIDGET_H

#include "img/voxelvolume.h"
#ifdef OCL_CONFIG_MODULE_AVAILABLE
#include "processing/volumeslicer.h"
#endif

#include <QWidget>

namespace Ui {
class VolumeSlicerWidget;
}

class VolumeSlicerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeSlicerWidget(QWidget *parent = 0);
    ~VolumeSlicerWidget();

private:
    Ui::VolumeSlicerWidget *ui;

#ifdef OCL_CONFIG_MODULE_AVAILABLE
public:
    void setData(const CTL::VoxelVolume<float>& volume);

public slots:
    void dataChange();
    void planeChange();

private:
    void recomputeSlice();
    std::unique_ptr<CTL::OCL::VolumeSlicer> _slicer;
#endif
};

#endif // VOLUMESLICERWIDGET_H