#include "chunk2dview.h"

#ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
#include "gui/widgets/lineseriesview.h"
#else
#include <QMessageBox>
#endif

#include <cmath>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsLineItem>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QMouseEvent>

namespace CTL {
namespace gui {

/*!
 * Creates a Chunk2DView and sets its parent widget to \a parent. Note that you need to call show()
 * to display the window.
 *
 * The static method plot() can be used as a convenience alternative for quick visualization.
 */
Chunk2DView::Chunk2DView(QWidget* parent)
    : QGraphicsView(parent)
    , _imageItem(new QGraphicsPixmapItem)
    , _contrastLineItem(new QGraphicsLineItem)
{
    setGrayscaleColorTable();

    setScene(&_scene);
    _scene.addItem(_imageItem);
    _scene.addItem(_contrastLineItem);
    _contrastLineItem->setPen(QPen(Qt::red));
    _contrastLineItem->hide();

    setBackgroundBrush(QBrush(checkerboard()));

    setMinimumSize({10, 10});
    setWindowTitle("Chunk2D view");
}

/*!
 * Creates a Chunk2DView with parent widget \a parent and sets its data to \a data. Note that you
 * need to call show() to display the window.
 *
 * The static method plot() can be used as a convenience alternative for quick visualization.
 */
Chunk2DView::Chunk2DView(Chunk2D<float> data, QWidget* parent)
    : Chunk2DView(parent)
{
    setData(std::move(data));
}

/*!
 * Creates a Chunk2DView for \a data and shows the window. If specific values are passed with
 * \a windowing and/or \a zoom, the data windowing and zoom are set to the requested values,
 * respectively. Otherwise, min/max windowing is applied and zoom remains at 1x.
 *
 * Sensitivity of windowing using mouse gestures is adapted automatically to \a data (see
 * setAutoMouseWindowScaling()).
 *
 * The widget will be deleted automatically if the window is closed.
 *
 * Example:
 * \code
 * // create a ball volume, filled with value 1.0
 * auto volume = VoxelVolume<float>::ball(100.0f, 1.0f, 1.0f);
 * // select slice 11 in *z*-direction
 * auto slice = volume.sliceZ(10);
 *
 * gui::Chunk2DView::plot(slice);
 * \endcode
 */
void Chunk2DView::plot(Chunk2D<float> data, QPair<double, double> windowing, double zoom)
{
    auto viewer = new Chunk2DView;

    viewer->setWindowing(windowing.first, windowing.second);
    viewer->setZoom(zoom);

    viewer->setData(std::move(data));
    viewer->autoResize();
    viewer->setAutoMouseWindowScaling();
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    viewer->show();
}

// setter

/*!
 * Sets the colormap of this instance to \a colorTable. The table must contain 256 entries.
 *
 * For visualization, data managed by this instance is discretized in 256 bins within the value
 * range specified by the current windowing settings. Each of these bins uses one color from the
 * colormap to visualize data points falling within that bin.
 *
 * Example:
 * \code
 * auto viewer = new gui::Chunk2DView;
 * // ...
 *
 * // create a colormap (here: gradient from black to red)
 * QVector<QRgb> blackRedMap(256);
 *  for(int i = 0; i <= 255; ++i)
 *      blackRedMap[i] = qRgb(i,0,0);
 *
 * // set colormap to the viewer
 * viewer->setColorTable(blackRedMap);
 * \endcode
 */
void Chunk2DView::setColorTable(const QVector<QRgb>& colorTable)
{
    if(_colorTable.size() != 256)
        qWarning() << "Setting colormap with inappropriate size. 256 values are required.";
    _colorTable = colorTable;

    updateImage();
}

/*!
 * Sets the data visualized by this instance to \a data. Data is copied, so consider moving it if
 * it is no longer required.
 *
 * Applies a min/max windowing if no specific windowing has been set (ie. the current window is
 * [0,0]).
 */
void Chunk2DView::setData(Chunk2D<float> data)
{
    _data = std::move(data);

    if(_window.first == 0 && _window.second == 0) // still default values -> window min/max
        setWindowingMinMax(); // this includes updateImage()
    else
        updateImage(); // keep previous window
}

/*!
 * Sets the scaling of windowing using mouse gestures.
 *
 * A vertical mouse movement of one pixel will raise/lower of the center (or level) of the current
 * window by \a centerScale.
 * A horizontal mouse movement of one pixel will result in an decrease/increase of the window width
 * of \a widthScale.
 */
void Chunk2DView::setMouseWindowingScaling(double centerScale, double widthScale)
{
    _mouseWindowingScaling.first  = centerScale;
    _mouseWindowingScaling.second = widthScale;
}

/*!
 * Sets the scaling of zooming commands using the mouse wheel (CTRL + wheel).
 *
 * The current zoom factor will be increased/decreased by \a zoomPerTurn per 15 degree rotation
 * of the wheel. Typically, one wheel step corresponds to 15 degrees of rotation.
 */
void Chunk2DView::setWheelZoomPerTurn(double zoomPerTurn)
{
    _wheelZoomPerTurn = zoomPerTurn;
}

/*!
 * Returns the data on the currently drawn contrast line (Right button + drag mouse).
 *
 * Data is returned as a list of points containing the position on the line (ranging from 0 to 1)
 * as *x* component and the corresponding data point as *y*. The line will be sampled with a step
 * width of one pixel.
 */
QList<QPointF> Chunk2DView::contrastLine() const
{
    QList<QPointF> ret;

    auto line = _contrastLineItem->line();
    const int nbSteps = qRound(line.length() + 0.5);
    const double step = 1.0 / double(nbSteps);

    const auto imageRect = QRect(0, 0, _data.width(), _data.height());
    for(int s = 0; s < nbSteps; ++s)
    {
        const auto par = s*step;
        const auto pixel = (line.pointAt(par) / _zoom).toPoint();
        const auto value = imageRect.contains(pixel) ? _data(pixel.x(), pixel.y())
                                                     : 0.0f;
        ret.append( { par, value });
    }

    return ret;
}

/*!
 * Returns the current visualization shown by this instance rendered to a QImage with size
 * \a renderSize. If no size is passed, the resulting image will have the same size as the window
 * this instance is shown in.
 */
QImage Chunk2DView::image(const QSize& renderSize)
{
    QSize imgSize = renderSize.isValid() ? renderSize : size();

    QImage ret(imgSize, QImage::Format_ARGB32);
    QPainter painter(&ret);

    render(&painter);

    return ret;
}

/*!
 * Sets the axis labels of contrast plots created by this instance to \a labelX and \a labelY.
 */
void Chunk2DView::setContrastLinePlotLabels(const QString& labelX, const QString& labelY)
{
    _contrLineLabelX = labelX;
    _contrLineLabelY = labelY;
}

/*!
 * Creates (and shows) a contrast plot of the currently drawn contrast line.
 *
 * Note that this requires the 'ctl_gui_charts.pri' submodule to be included to the project.
 */
void Chunk2DView::showContrastLinePlot()
{
#ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
    LineSeriesView::plot(contrastLine(), _contrLineLabelX, _contrLineLabelY);
#else
    QMessageBox::information(this, "Contrast line plot", "Contrast line plot not available.\n"
                                                         "(Requires 'gui_widgets_charts.pri' submodule.)");
#endif
}


// getter
/*!
 * Returns the data held by this instance.
 */
const Chunk2D<float>& Chunk2DView::data() const { return _data; }

/*!
 * Returns the currently shown pixmap.
 */
QPixmap Chunk2DView::pixmap() const { return _imageItem->pixmap(); }

/*!
 * Returns the current data windowing as a pair specifying the window start and end point.
 */
QPair<double, double> Chunk2DView::windowingFromTo() const { return _window; }

/*!
 * Returns the current data windowing as a pair specifying the window center and width.
 */
QPair<double, double> Chunk2DView::windowingCenterWidth() const
{
    const auto width = _window.second - _window.first;
    const auto center = _window.first + width / 2.0;

    return qMakePair(center, width);
}

/*!
 * Returns the current zoom factor. The value 1.0 corresponds to a one-by-one visualization (ie.
 * 100% zoom).
 */
double Chunk2DView::zoom() const { return _zoom; }

// slots
/*!
 * Requests an automatic resizing of this widget's window size. The window is tried to fit to the
 * size of the shown data, bounded to a maximum size of 1000 x 800 pixels.
 */
void Chunk2DView::autoResize()
{
    static const auto maxSize = QSize(1000, 800);
    static const auto margins = QSize(2, 2);

    const auto imgSize = QSize(_data.width(), _data.height()) + margins;

    resize(imgSize.boundedTo(maxSize));
}

/*!
 * Sets the broadcasting of live pixel data by this instance to \a enabled.
 *
 * If enabled, a signal is emitted each time the mouse cursor moves over the image, containing the
 * pixel coordinates and the corresponding data value under the cursor.
 *
 * This signal can be catched and processed elsewhere.
 */
void Chunk2DView::setLivePixelDataEnabled(bool enabled) { setMouseTracking(enabled); }

/*!
 * Sets the data windowing to show the value range [\a from, \a to] using the current colormap.
 */
void Chunk2DView::setWindowing(double from, double to)
{
    if(from > to)
    {
        qWarning() << "Windowing start must not be larger that its end.";
        return;
    }

    _window.first  = from;
    _window.second = to;
    updateImage();

    emit windowingChanged(from, to);
}

/*!
 * Sets the data windowing to show the entire value range (ie. minimum to maximum) occurring in the
 * data managed by this instance.
 */
void Chunk2DView::setWindowingMinMax()
{
    const auto dataMin = static_cast<double>(_data.min());
    const auto dataMax = static_cast<double>(_data.max());

    setWindowing(dataMin, dataMax);
}

/*!
 * Sets the data windowing to show a value range with a width of \a width centered around \a center
 * using the current colormap.
 *
 * In terms of start and end point, this corresponds to a window of [\a center - \a width / 2.0,
 * \a center + \a width / 2.0].
 */
void Chunk2DView::setWindowingCenterWidth(double center, double width)
{
    const auto from  = center - width / 2.0;
    const auto to = center + width / 2.0;

    setWindowing(from, to);
}

/*!
 * Returns the zoom factor to \a zoom. The value 1.0 corresponds to a one-by-one visualization (ie.
 * 100% zoom).
 *
 * Zoom may not be smaller than 0.1 (ie. zoom level of 10%).
 */
void Chunk2DView::setZoom(double zoom)
{
    if(zoom < 0.1)
    {
        qWarning() << "Zoom factor too small. It will be ignored.";
        return;
    }

    _zoom = zoom;
    updateImage();

    emit zoomChanged(zoom);
}

/*!
 * Sets the scaling of windowing using mouse gestures to automatically determined values that are
 * optimized for the value range in the currently managed data.
 *
 * The sensitivity is adjusted such that, given a total value range in the data of [min, max], mouse
 * gestures have the following effects:
 * - A vertical mouse movement of one pixel will raise/lower of the center (or level) of the current
 * window by 1% of the total value range (ie. max - min).
 * A horizontal mouse movement of one pixel will result in an decrease/increase of the window width
 * of 1% of the total value range (ie. max - min).
 */
void Chunk2DView::setAutoMouseWindowScaling()
{
    static const auto percentageOfFull = 0.01;

    const auto dataMin = static_cast<double>(_data.min());
    const auto dataMax = static_cast<double>(_data.max());

    const auto dataWidth = dataMax - dataMin;

    setMouseWindowingScaling(percentageOfFull * dataWidth, percentageOfFull * dataWidth);
}

// other slots
/*!
 * Saves the image currently shown by this instance to the file \a fileName.
 *
 * The file type must be an image file type supported by Qt and will be determined automatically
 * from the ending of \a fileName. If no file type ending is found, or it is incompatible, a PNG
 * file is created.
 *
 * Same as: \code image().save(fileName) \endcode
 */
bool Chunk2DView::save(const QString& fileName)
{
    return image().save(fileName);
}

/*!
 * Opens a save file dialog to get the file name used to save the currently shown image to a file.
 *
 * \sa save().
 */
void Chunk2DView::saveDialog()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save plot", "", "Images (*.png *.jpg *.bmp)");
    if(fn.isEmpty())
        return;

    save(fn);
}

// event handling

void Chunk2DView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_K)
    {
        showContrastLinePlot();
        event->accept();
        return;
    }
    else if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        saveDialog();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void Chunk2DView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        setWindowingMinMax();
        event->accept();
    }

    QWidget::mouseDoubleClickEvent(event);
}

void Chunk2DView::mouseMoveEvent(QMouseEvent* event)
{   
    if(event->buttons() == Qt::LeftButton)
    {        
        const auto dragVector = event->pos() - _mouseDragStart;

        auto centerAdjust = -dragVector.y() * _mouseWindowingScaling.first;
        auto widthAdjust  = dragVector.x() * _mouseWindowingScaling.second;

        setWindowingCenterWidth(_windowDragStartValue.first + centerAdjust,
                                _windowDragStartValue.second + widthAdjust);
    }
    if(event->buttons() == Qt::RightButton)
    {
        _contrastLineItem->show();
        _contrastLineItem->setLine( { mapToScene(_mouseDragStart), mapToScene(event->pos()) } );
    }
    else if(itemAt(event->pos()) == _imageItem)
    {
        const auto pixel = pixelIdxFromPos(event->pos());
        emit pixelInfoUnderCursor(pixel.x(), pixel.y(), _data(pixel.x(), pixel.y()));
    }

    QWidget::mouseMoveEvent(event);
}

void Chunk2DView::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        _mouseDragStart = event->pos();
        _windowDragStartValue = windowingCenterWidth();
    }
    else if(event->button() == Qt::RightButton)
    {
        _mouseDragStart = event->pos();
        _contrastLineItem->setLine( { mapToScene(_mouseDragStart), mapToScene(_mouseDragStart) } );
        _contrastLineItem->hide();
    }

    QWidget::mousePressEvent(event);
}

void Chunk2DView::wheelEvent(QWheelEvent* event)
{
    if(event->modifiers() == Qt::CTRL)
    {
        const QPoint numTurns = event->angleDelta() / 120.0; // in steps of 15 deg
        setZoom(_zoom + numTurns.y() * _wheelZoomPerTurn);
        event->accept();
    }
    else
        QGraphicsView::wheelEvent(event);
}

// private methods

QPoint Chunk2DView::pixelIdxFromPos(const QPoint& pos)
{
    static const QPointF halfPixel = {0.5, 0.5};
    return (mapToScene(pos) / _zoom - halfPixel).toPoint();
}

QPixmap Chunk2DView::checkerboard() const
{
    auto colorTable = QVector<QRgb>(256);

    for(int i = 0; i <= 255; ++i)
        colorTable[i] = qRgb(i,i,i);

    QImage img(20, 20, QImage::Format_Indexed8);
    img.setColorTable(colorTable);

    for(int i = 0; i < 10; ++i)
    {
        for(int j = 0; j < 10; ++j)
            img.setPixel(i, j, 100);
        for(int j = 10; j < 20; ++j)
            img.setPixel(i, j, 150);
    }
    for(int i = 10; i < 20; ++i)
    {
        for(int j = 0; j < 10; ++j)
            img.setPixel(i, j, 150);
        for(int j = 10; j < 20; ++j)
            img.setPixel(i, j, 100);
    }

    return QPixmap::fromImage(img);
}

void Chunk2DView::setGrayscaleColorTable()
{
    _colorTable = QVector<QRgb>(256);

    for(int i = 0; i <= 255; ++i)
        _colorTable[i] = qRgb(i,i,i);
}

void Chunk2DView::updateImage()
{
    const auto imgWidth  = int(_data.width());
    const auto imgHeight = int(_data.height());

    QImage image(imgWidth, imgHeight, QImage::Format_Indexed8);
    image.setColorTable(_colorTable);

    const auto minGrayValue = static_cast<float>(_window.first);
    const auto maxGrayValue = static_cast<float>(_window.second);
    const auto grayScale = 255.0f / float(maxGrayValue - minGrayValue);
    const auto offset = - minGrayValue * grayScale + 0.5f; // 0.5 for rounding

    auto dataIt = _data.data().cbegin();
    for(int y = 0; y < imgHeight; ++y)
    {
        auto linePtr = image.scanLine(y);
        for(int x = 0; x < imgWidth; ++x)
        {
            *linePtr = static_cast<uchar>(qMax(qMin( // clamp to interval [0, 255]
                std::fma(*dataIt, grayScale, offset), 255.f), 0.f)); // projVal * grayScale + offset
            ++linePtr;
            ++dataIt;
        }
    }

    auto pixmap = QPixmap::fromImage(image).scaledToHeight(qRound(imgHeight * _zoom));
    _imageItem->setPixmap(pixmap);
    _scene.setSceneRect(QRectF({0, 0}, pixmap.size()));
}

} // namespace gui
} // namespace CTL
