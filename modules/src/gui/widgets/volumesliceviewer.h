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

/*!
 * \class VolumeSliceViewer
 *
 * \brief The VolumeSliceViewer class visualizes data from a volume sampled along arbitrary planes
 * through the volume.
 *
 * This class can be used to visualize from a plane intersecting a given volume. Volume data to
 * visualize is set via setData(). This class uses IntersectionPlaneView to provide a preview of the
 * intersection plane in a 3D viewer. For this to work, the gui_widgets_3d.pri submodule is
 * available. If unavailable, no preview will be shown.
 *
 * For convenience, the plot() method can be used to achieve a one-line solution, creating a
 * widget that will be destroyed once it is closed by the user.
 *
 * The following IO operations are supported by this widget:
 *
 * Within the viewport of the intersection plane visualization:
 * - Zooming:
 *    - Scroll mouse wheel up/down to zoom in/out.
 * - Camera positioning / orientation:
 *    - Hold left mouse button + move up/down/left/right to move the camera position in the
 * corresponding direction
 *    - Hold right mouse button + move up/down/left/right to rotate the camera direction
 *
 * Within the viewport of the sampled slice:
 * - Change slice:
 *    - Hold SHIFT + scroll mouse wheel to up/down to show next/previous slice
 * - Zooming:
 *    - Hold CTRL + scroll mouse wheel up/down to zoom in/out.
 * - Data windowing:
 *    - Hold left mouse button + move up/down to raise/lower the center (or level) of the window.
 *    - Hold left mouse button + move left/right to narrow/broaden the width of the window.
 *    - Double-click left to request automatic windowing (ie. min/max-window).
 * - Plotting a contrast line:
 *    - Hold right mouse button + drag mouse to draw a line.
 *    - Press 'K' button to create a contrast line plot of the current line (requires
 * ctl_gui_charts.pri submodule).
 *    - Press CTRL + C to copy the currently drawn contrast line coordinates to the clipboard
 *    - Press CTRL + V to set a contrast line based on previously copied coordinates from the
 * clipboard. The coordinates can also be copied from another window or widget.
 * - Read-out live pixel data under cursor:
 *    - Mouse movements: Live pixel data is shown under the bottom right corner of the image.
 * - Save to image:
 *    - Press CTRL + S to open a dialog for saving the current figure to a file. (Note that only the
 * slice viewer part can be saved. Saving the 3D intersection plane view is not supported.)
 * - Plotting a contrast line:
 *    - Press 'K' button to create a contrast line plot of the current line (requires
 * ctl_gui_charts.pri submodule).
 *
 * Example:
 * \code
 * auto volume = VoxelVolume<float>::cylinderZ(50.0f, 200.0f, 1.0f, 1.0f);
 *
 * gui::VolumeSliceViewer::plot(volume);
 * \endcode
 *
 * ![Visualization from the example above for a specific plane (135°, 55°, -25mm) through the cylinder.](gui/VolumeSliceViewer.png)
 */

class VolumeSliceViewer : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeSliceViewer(QWidget* parent = nullptr);
    ~VolumeSliceViewer();

    void setData(const VoxelVolume<float>& volume);

    static void plot(const VoxelVolume<float>& volume);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::VolumeSliceViewer *ui;
    std::unique_ptr<OCL::VolumeSlicer> _slicer;
    IntersectionPlaneView* _3dViewer = nullptr;

    void dataChange();
    void recomputeSlice();

private slots:
    void planeChange();
    void updatePixelInfo(int x, int y, float value);
    void windowingUpdate();
};

} // namespace gui
} // namespace CTL

#endif // CTL_VOLUMESLICEVIEWER_H
