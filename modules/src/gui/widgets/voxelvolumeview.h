#ifndef VOXELVOLUMEVIEW_H
#define VOXELVOLUMEVIEW_H

#include <QWidget>
#include <QDebug>

#include "img/voxelvolume.h"

namespace Ui {
class VoxelVolumeView;
}

class VoxelVolumeView : public QWidget
{
    Q_OBJECT

public:
    explicit VoxelVolumeView(QWidget *parent = nullptr);
    ~VoxelVolumeView();

    template<typename T>
    void setVolumeData(const CTL::VoxelVolume<T>& volume);

private slots:
    void on_verticalSlider_valueChanged(int value);
    void updateImage();
    void updateSliderRange();
    void autoWindowing();

private:
    Ui::VoxelVolumeView *ui;

    CTL::VoxelVolume<float> _data = CTL::VoxelVolume<float>(0,0,0);
    QVector<QRgb> _colorTable;

    void setColorTable();
    void checkIfAutoWindowingRequired();
};

template<typename T>
void VoxelVolumeView::setVolumeData(const CTL::VoxelVolume<T> &volume)
{
    _data = CTL::VoxelVolume<float>(volume.nbVoxels().x,
                                    volume.nbVoxels().y,
                                    volume.nbVoxels().z);

    _data.allocateMemory();
    auto dataPtr = _data.rawData();
    auto srcData = volume.constData();

    for(size_t vox=0, total = volume.totalVoxelCount(); vox < total; ++vox)
        dataPtr[vox] = static_cast<float>(srcData[vox]);

    updateSliderRange();
    checkIfAutoWindowingRequired();
}

#endif // VOXELVOLUMEVIEW_H
