#ifndef CTL_VOLUMESLICERWIDGET_H
#define CTL_VOLUMESLICERWIDGET_H

#include "img/voxelvolume.h"
#ifdef OCL_CONFIG_MODULE_AVAILABLE
#include "processing/volumeslicer.h"
#endif
#ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
    #include "gui/widgets/lineseriesview.h"
#endif

#include <QWidget>

namespace Ui {
class VolumeSlicerWidget;
}

namespace CTL {
namespace gui {

class VolumeSlicerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeSlicerWidget(QWidget *parent = nullptr);
    ~VolumeSlicerWidget();

private:
    Ui::VolumeSlicerWidget *ui;

#ifdef OCL_CONFIG_MODULE_AVAILABLE
public:
    void setData(const CTL::VoxelVolume<float>& volume);

public slots:
    void dataChange();
    void planeChange();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void recomputeSlice();
    std::unique_ptr<CTL::OCL::VolumeSlicer> _slicer;

private slots:
    void updatePixelInfo(int x, int y, float value);
    void windowingUpdate();
#endif
};

} // namespace gui
} // namespace CTL

#endif // CTL_VOLUMESLICERWIDGET_H
