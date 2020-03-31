#ifndef CTL_VOLUMESLICEVIEWER_H
#define CTL_VOLUMESLICEVIEWER_H

#include "img/voxelvolume.h"
#include "processing/volumeslicer.h"

#include <QWidget>

namespace Ui {
class VolumeSliceViewer;
}

namespace CTL {
namespace gui {

class IntersectionPlaneView;

class VolumeSliceViewer : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeSliceViewer(QWidget *parent = nullptr);
    ~VolumeSliceViewer();

    void setData(const VoxelVolume<float>& volume);

    static void plot(const VoxelVolume<float>& volume);

public slots:
    void dataChange();
    void planeChange();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::VolumeSliceViewer *ui;

    void recomputeSlice();
    std::unique_ptr<OCL::VolumeSlicer> _slicer;
    IntersectionPlaneView* _3dViewer = nullptr;

private slots:
    void updatePixelInfo(int x, int y, float value);
    void windowingUpdate();
};

} // namespace gui
} // namespace CTL

#endif // CTL_VOLUMESLICEVIEWER_H
