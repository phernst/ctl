#ifndef CTL_VOLUMEVIEWER_H
#define CTL_VOLUMEVIEWER_H

#include <QWidget>
#include "img/compositevolume.h"

namespace Ui {
class VolumeViewer;
}

namespace CTL {
namespace gui {

class Chunk2DView;

/*!
 * \class VolumeViewer
 *
 * \brief The VolumeViewer class provides a tool for visualization of volume data.
 *
 * This class can be used to visualize different volume data objects. It fully supports:
 * - VoxelVolume<float>
 * - SpectralVolumeData
 * - CompositeVolume
 * Any subclass of AbstractDynamicVolumeData can be visualized as well, but will be limited to
 * displaying the current state of the volume (ie. as set by AbstractDynamicVolumeData::setTime()).
 *
 * For convenience, the plot() method can be used to achieve a one-line solution, creating a
 * widget that will be destroyed once it is closed by the user.
 *
 * ![A VolumeViewer window visualizing data of a CompositeVolume containing two subvolumes: a water cylinder (currently shown) a ball.](gui/VolumeViewer.png)
 *
 * Use the vertical slider to cylce through the slices of the current volume. When visualizing a
 * CompositeVolume, the table widget on the right shows an overview over its different subvolumes.
 * Click on a row to visualize the corresponding subvolume in the viewport. If desired (e.g. to save
 * screen space), the subvolume overview can be hidden with hideCompositeOverview). Subvolumes can
 * then only be selected using showSubvolume().
 *
 * The following IO operations are supported by this widget:
 *
 * Within the viewport of the current slice:
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
 *
 * Anywhere in the widget:
 * - Save to image:
 *    - Press CTRL + S to open a dialog for saving the current figure to a file.
 * - Plotting a contrast line:
 *    - Press 'K' button to create a contrast line plot of the current line (requires
 * ctl_gui_charts.pri submodule).
 *
 * Contrast line plots are opened in separate windows (see LineSeriesView for details on the
 * corresponding widget).
 *
 * Visualization of the slice image itself is done using the Chunk2DView class. The viewport can be
 * accessed with dataView(), in order to adjust its settings. For mouse gesture windowing, a
 * convenience method setAutoMouseWindowScaling() exists to directly set a sensitivity suited
 * for the current data.
 *
 * Example:
 * \code
 * // Creating two volumes
 * auto cylinderVol = SpectralVolumeData::cylinderX(30.0f, 100.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Water));
 * auto ballVolume  = SpectralVolumeData::ball(10.0f, 1.0f, 1.0f, attenuationModel(database::Composite::Blood));
 *
 * // visualize a single volume (static version)
 * gui::VolumeViewer::plot(cylinderVol);
 *
 * //create a CompositeVolume and visualize it (property-based version)
 * CompositeVolume compositeVol(std::move(cylinderVol), std::move(ballVolume));
 *
 * auto viewer = new gui::VolumeViewer; // needs to be deleted at an appropriate time
 * viewer->setData(compositeVol);
 * viewer->autoResize();
 * viewer->show();
 * \endcode
 *
 * Note that it is absolutely possible to use the static version also to visualize a
 * CompositeVolume. However, he property-based version provides more flexibility as it gives access
 * to the full set of methods to configure the VolumeViewer. In particular, it allows us to
 * configure the viewport showing the individual slices. The following example shows, how we can use
 * this approach to create a visualization that uses a black-red colormap:
 * \code
 * // create colormap
 * QVector<QRgb> blackRedMap(256);
 *  for(int i = 0; i <= 255; ++i)
 *      blackRedMap[i] = qRgb(i,0,0);
 *
 * auto viewer = new gui::VolumeViewer; // needs to be deleted at an appropriate time
 * viewer->setData(compositeVol);       // see example above for 'compositeVol'
 * viewer->dataView()->setColorTable(blackRedMap);
 *
 * viewer->autoResize();
 * viewer->show();
 * \endcode
 */
class VolumeViewer : public QWidget
{
    Q_OBJECT

public:
    enum class WindowPreset {
        Abdomen,
        Angio,
        Bone,
        Brain,
        Chest,
        Lungs
    };

    explicit VolumeViewer(QWidget *parent = nullptr);
    VolumeViewer(CompositeVolume volume, QWidget *parent = nullptr);
    ~VolumeViewer();

    static void plot(CompositeVolume data);
    static void plot(SpectralVolumeData data);

    const CompositeVolume& data() const;
    Chunk2DView* dataView() const;
    void setData(SpectralVolumeData data);
    void setData(CompositeVolume data);
    void setWindowPresets(WindowPreset preset1, WindowPreset preset2);
    void setWindowPresetsInMu(WindowPreset preset1, WindowPreset preset2, float referenceEnergy);
    void setWindowPresets(QPair<QString, QPair<double, double>> preset1,
                          QPair<QString, QPair<double, double>> preset2);

public slots:
    void autoResize();
    void hideCompositeOverview(bool hide = true);
    void setAutoMouseWindowScaling();
    void showSlice(int slice);
    void showSubvolume(int subvolume);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::VolumeViewer *ui;

    CompositeVolume _compData;

    const SpectralVolumeData& selectedVolume() const;

private slots:
    void selectCentralSlice();
    void sliceDirectionChanged();
    void updateVolumeOverview();
    void updateSliderRange();
    void updatePixelInfo(int x, int y, float value);
    void volumeSelectionChanged();
    void windowingUpdate();
};

} // namespace gui
} // namespace CTL

#endif // CTL_VOLUMEVIEWER_H
