#ifndef CTL_VOLUMESLICERWIDGET_H
#define CTL_VOLUMESLICERWIDGET_H

#include "img/voxelvolume.h"
#include "processing/volumeslicer.h"

#include <QWidget>

namespace Ui {
class VolumeSlicerWidget;
}

namespace CTL {
namespace gui {

class PlaneVisualizer;

class VolumeSlicerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeSlicerWidget(QWidget *parent = nullptr);
    ~VolumeSlicerWidget();

    void setData(const CTL::VoxelVolume<float>& volume);

public slots:
    void dataChange();
    void planeChange();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::VolumeSlicerWidget *ui;

    void recomputeSlice();
    std::unique_ptr<CTL::OCL::VolumeSlicer> _slicer;
    PlaneVisualizer* _3dViewer = nullptr;

private slots:
    void updatePixelInfo(int x, int y, float value);
    void windowingUpdate();
};

} // namespace gui
} // namespace CTL

#endif // CTL_VOLUMESLICERWIDGET_H
