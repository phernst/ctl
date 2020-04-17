#ifndef CTL_CHUNK2DVIEW_H
#define CTL_CHUNK2DVIEW_H

#include <QGraphicsView>

#include "img/chunk2d.h"

class QLabel;

namespace CTL {
namespace gui {

/*!
 * \class Chunk2DView
 *
 * \brief The Chunk2DView class provides basic visualization of Chunk2D data.
 *
 * This class can be used to visualize data stored in a Chunk2D. For convenience, the plot() method
 * can be used to achieve a one-line solution, creating a widget that will be destroyed once it is
 * closed by the user.
 *
 * ![A Chunk2DView window showing a projection image of two balls and a cylinder.](gui/Chunk2DView.png)
 *
 * Data will be visualized in 256 discrete value steps - according to the current window - using the
 * colormap specified by setColorTable(). By default, a grayscale colormap is used.
 *
 * The following IO operations are supported by this class:
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
 * - Save to image:
 *    - Press CTRL + S to open a dialog for saving the current figure to a file.
 * - Read-out live pixel data under cursor:
 *    - Can be enabled with setLivePixelDataEnabled(true); if enabled, emits a signal with pixel
 * information that can be catched elsewhere.
 *
 * Sensitivity of mouse gestures (move, wheel) can be controlled with setMouseWindowingScaling() and
 * setWheelZoomPerTurn(). For windowing, a convenience method setAutoMouseWindowScaling() exists to
 * set a sensitivity suited for the current data.
 *
 * The following example shows how to visualize a single slice from a VoxelVolume data object using
 * the Chunk2DView:
 * \code
 * // create a ball volume, filled with value 1.0
 * auto volume = VoxelVolume<float>::ball(100.0f, 1.0f, 1.0f);
 * // select slice 11 in *z*-direction
 * auto slice = volume.sliceZ(10);
 *
 * // (static version) using the plot() command
 * gui::Chunk2DView::plot(slice);
 *
 * // (property-based version)
 * auto viewer = new gui::Chunk2DView; // needs to be deleted at an appropriate time
 * viewer->setData(slice);
 * viewer->autoResize();
 * viewer->show();
 * \endcode
 *
 * The property-based version provides more flexibility as it gives access to the full set of
 * methods to configure the Chunk2DView. The following example shows, how we can use this approach
 * to create a visualization that uses a black-red colormap:
 * \code
 * // create colormap
 * QVector<QRgb> blackRedMap(256);
 *  for(int i = 0; i <= 255; ++i)
 *      blackRedMap[i] = qRgb(i,0,0);
 *
 * auto viewer = new gui::Chunk2DView; // needs to be deleted at an appropriate time
 * viewer->setData(slice);             // see above example for 'slice'
 * viewer->autoResize();
 * viewer->setColorTable(blackRedMap);
 * viewer->show();
 * \endcode
 */

class Chunk2DView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit Chunk2DView(QWidget* parent = nullptr);
    Chunk2DView(Chunk2D<float> data, QWidget* parent = nullptr);
    //~Chunk2DView();

    // factory
    static void plot(Chunk2D<float> data,
                     QPair<double,double> windowing = qMakePair(0.0, 0.0), double zoom = 1.0);

    // getter methods
    const Chunk2D<float>& data() const;
    QPixmap pixmap() const;
    QPair<double, double> windowingFromTo() const;
    QPair<double, double> windowingCenterWidth() const;
    double zoom() const;

    // setter methods
    void setColorTable(const QVector<QRgb>& colorTable);
    template<typename T>
    void setData(const Chunk2D<T>& data);
    void setData(Chunk2D<float>&& data);
    void setMouseWindowingScaling(double centerScale, double widthScale);
    void setWheelZoomPerTurn(double zoomPerTurn);

    QList<QPointF> contrastLine() const;
    QImage image(const QSize& renderSize = QSize());
    void setContrastLinePlotLabels(const QString& labelX, const QString& labelY);
    void showContrastLinePlot();

public slots:
    void autoResize();
    bool save(const QString& fileName);
    void saveDialog();
    void setAutoMouseWindowScaling();
    void setLivePixelDataEnabled(bool enabled);
    void setWindowing(double from, double to);
    void setWindowingCenterWidth(double center, double width);
    void setWindowingMinMax();
    void setZoom(double zoom);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

signals:
    void pixelInfoUnderCursor(int x, int y, float value);
    void viewChangeRequested(int requestedChange);
    void windowingChanged(double from, double to);
    void zoomChanged(double zoom);

private:
    QGraphicsScene _scene;
    QGraphicsPixmapItem* _imageItem;
    QGraphicsLineItem* _contrastLineItem;

    Chunk2D<float> _data = Chunk2D<float>(0,0);
    QVector<QRgb> _colorTable;
    QPair<double, double> _window;
    double _zoom = 1.0;
    QString _contrLineLabelX = QStringLiteral("Position on line");
    QString _contrLineLabelY = QStringLiteral("Value");

    // event handling
    QPoint _mouseDragStart;
    QPair<double, double> _windowDragStartValue;
    QPair<double, double> _mouseWindowingScaling = {1.0, 1.0};
    double _wheelZoomPerTurn = 0.25; // i.e 0.25 zoom per 15.0 deg;

    QPixmap checkerboard() const;
    void contrastLineFromClipbord();
    void contrastLineToClipbord() const;
    QPoint pixelIdxFromPos(const QPoint& pos);
    void setGrayscaleColorTable();
    void updateImage();
};

template<typename T>
void Chunk2DView::setData(const Chunk2D<T>& data)
{
    Chunk2D<float> convChunk(data.dimensions());
    convChunk.allocateMemory();

    const auto& dataRef = data.constData();
    std::transform(dataRef.cbegin(), dataRef.cend(), convChunk.data().begin(),
                   [] (const T& val) { return static_cast<float>(val); } );

    setData(std::move(convChunk));
}

template<> void Chunk2DView::setData<float>(const Chunk2D<float>& data);

} // namespace gui
} // namespace CTL

#endif // CTL_CHUNK2DVIEW_H
