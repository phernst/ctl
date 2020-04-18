#ifndef CTL_PROJECTIONVIEWER_H
#define CTL_PROJECTIONVIEWER_H

#include <QWidget>

#include "img/projectiondata.h"

namespace Ui {
class ProjectionViewer;
}

namespace CTL {
namespace gui {

class Chunk2DView;

/*!
 * \class ProjectionViewer
 *
 * \brief The ProjectionViewer class provides a tool for visualization of ProjectionData.
 *
 * This class can be used to visualize ProjectionData objects. For convenience, the plot() method
 * can be used to achieve a one-line solution, creating a widget that will be destroyed once it is
 * closed by the user.
 *
 * ![A ProjectionViewer window visualizing projection data of two balls and a cylinder. A line has been drawn across the center of the projection, for which a contrast plot can be requested pressing 'K'.](gui/ProjectionViewer_withCtrLine.png)
 *
 * Use the vertical slider to cycle through the projections from individual views. When projection
 * data shall be visualized that contains multiple detector modules, the specific ModuleLayout of
 * the detector configuration can be passed by setModuleLayout() and the projections will be
 * combined accordingly. Without additional specification of a layout, all found modules are assumed
 * to be arranged in a line next to each other in a horizontal manner.
 *
 * The following IO operations are supported by this widget:
 *
 * Within the viewport of the current projection:
 * - Change view:
 *    - Hold SHIFT + scroll mouse wheel to up/down to show next/previous view
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
 * ![Contrast line plot of the image shown on the top of this section.](gui/CtrLine.png)
 *
 * Visualization of the projection image itself is done using the Chunk2DView class. The viewport
 * can be accessed with dataView(), in order to adjust its settings. For mouse gesture windowing,
 * a convenience method setAutoMouseWindowScaling() exists to directly set a sensitivity suited
 * for the current data.
 *
 * Example:
 * \code
 * // Creating some projections
 * // define volume and acquisition setup (incl. system)
 * auto volume = VolumeData::cube(100, 1.0f, 0.02f);
 * auto system = CTSystemBuilder::createFromBlueprint(blueprints::GenericCarmCT(DetectorBinning::Binning4x4));
 *
 * AcquisitionSetup acquisitionSetup(system, 10);
 * acquisitionSetup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0));
 *
 * auto projector = new RayCasterProjector;
 * projector->configure(acquisitionSetup);
 * auto projections = projector->project(volume);
 *
 * // visualize results (static version)
 * gui::ProjectionViewer::plot(projections);
 *
 * // visualize results (property-based version)
 * auto viewer = new gui::ProjectionViewer; // needs to be deleted at an appropriate time
 * viewer->setData(projections);
 * viewer->autoResize();
 * viewer->show();
 * \endcode
 *
 * The property-based version provides more flexibility as it gives access to the full set of
 * methods to configure the ProjectionViewer. In particular, it allows us to configure the viewport
 * showing the individual projections. The following example shows, how we can use this approach
 * to create a visualization that uses a black-red colormap:
 * \code
 * // create colormap
 * QVector<QRgb> blackRedMap(256);
 *  for(int i = 0; i <= 255; ++i)
 *      blackRedMap[i] = qRgb(i,0,0);
 *
 * auto viewer = new gui::ProjectionViewer; // needs to be deleted at an appropriate time
 * viewer->setData(projections);            // see example above for 'projections'
 * viewer->dataView()->setColorTable(blackRedMap);
 *
 * viewer->autoResize();
 * viewer->show();
 * \endcode
 */

class ProjectionViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectionViewer(QWidget *parent = nullptr);
    ProjectionViewer(ProjectionData projections, QWidget *parent = nullptr);
    ~ProjectionViewer();

    static void plot(ProjectionData projections, const ModuleLayout& layout = {});

    const ProjectionData& data() const;
    Chunk2DView* dataView() const;
    void setData(ProjectionData projections);
    void setModuleLayout(const ModuleLayout& layout);
    void setWindowPresets(QPair<QString, QPair<double, double>> preset1,
                          QPair<QString, QPair<double, double>> preset2);

    int currentView() const;

public slots:
    void autoResize();
    void setAutoMouseWindowScaling();
    void showView(int view);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::ProjectionViewer *ui;

    ProjectionData _data = ProjectionData(0,0,0);
    ModuleLayout _modLayout;

private slots:
    void changeView(int requestedChange);
    void updateSliderRange();
    void updatePixelInfo(int x, int y, float value);
    void windowingUpdate();
};

} // namespace gui
} // namespace CTL

#endif // CTL_PROJECTIONVIEWER_H
