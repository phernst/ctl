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

Chunk2DView::Chunk2DView(Chunk2D<float> data, QWidget* parent)
    : Chunk2DView(parent)
{
    setData(std::move(data));
}

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

void Chunk2DView::setColorTable(const QVector<QRgb>& colorTable)
{
    _colorTable = colorTable;

    updateImage();
}

void Chunk2DView::setData(Chunk2D<float> data)
{
    _data = std::move(data);

    if(_window.first == 0 && _window.second == 0) // still default values -> window min/max
        setWindowingMinMax();
    else
        updateImage(); // keep previous window
}

void Chunk2DView::setMouseWindowingScaling(double centerScale, double widthScale)
{
    _mouseWindowingScaling.first  = centerScale;
    _mouseWindowingScaling.second = widthScale;
}

void Chunk2DView::setWheelZoomPerTurn(double zoomPerTurn)
{
    _wheelZoomPerTurn = zoomPerTurn;
}

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

QImage Chunk2DView::image(const QSize& renderSize)
{
    QSize imgSize = renderSize.isValid() ? renderSize : size();

    QImage ret(imgSize, QImage::Format_ARGB32);
    QPainter painter(&ret);

    render(&painter);

    return ret;
}

void Chunk2DView::setContrastLinePlotLabels(const QString& labelX, const QString& labelY)
{
    _contrLineLabelX = labelX;
    _contrLineLabelY = labelY;
}

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

const Chunk2D<float>& Chunk2DView::data() const { return _data; }

QPixmap Chunk2DView::pixmap() const { return _imageItem->pixmap(); }

QPair<double, double> Chunk2DView::windowingFromTo() const { return _window; }

QPair<double, double> Chunk2DView::windowingCenterWidth() const
{
    const auto width = _window.second - _window.first;
    const auto center = _window.first + width / 2.0;

    return qMakePair(center, width);
}

double Chunk2DView::zoom() const { return _zoom; }

// slots
void Chunk2DView::autoResize()
{
    static const auto maxSize = QSize(1000, 800);
    static const auto margins = QSize(2, 2);

    const auto imgSize = QSize(_data.width(), _data.height()) + margins;

    resize(imgSize.boundedTo(maxSize));
}

void Chunk2DView::setLivePixelDataEnabled(bool enabled) { setMouseTracking(enabled); }

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

void Chunk2DView::setWindowingMinMax()
{
    const auto dataMin = static_cast<double>(_data.min());
    const auto dataMax = static_cast<double>(_data.max());

    setWindowing(dataMin, dataMax);
}

void Chunk2DView::setWindowingCenterWidth(double center, double width)
{
    const auto from  = center - width / 2.0;
    const auto to = center + width / 2.0;

    setWindowing(from, to);
}

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

bool Chunk2DView::save(const QString& fileName)
{
    return image().save(fileName);
}

void Chunk2DView::saveDialog()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save plot", "", "Images (*.png *.jpg *.bmp)");
    if(fn.isEmpty())
        return;

    save(fn);
}

void Chunk2DView::setAutoMouseWindowScaling()
{
    static const auto percentageOfFull = 0.01;

    const auto dataMin = static_cast<double>(_data.min());
    const auto dataMax = static_cast<double>(_data.max());

    const auto dataWidth = dataMax - dataMin;
    const auto dataCenter = dataMin + dataWidth / 2.0;

    setMouseWindowingScaling(percentageOfFull * dataCenter, percentageOfFull * dataWidth);
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
