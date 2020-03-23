#include "chunk2dview.h"
#include "ui_chunk2dview.h"

#include <qevent.h>
#include <QDebug>


namespace CTL {
namespace gui {

Chunk2DView::Chunk2DView(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Chunk2DView)
{
    setGrayscaleColorTable();

    ui->setupUi(this);
}

Chunk2DView::Chunk2DView(Chunk2D<float> data, QWidget* parent)
    : Chunk2DView(parent)
{
    setData(std::move(data));
}

Chunk2DView::~Chunk2DView()
{
    delete ui;
}

void Chunk2DView::setColorTable(const QVector<QRgb>& colorTable)
{
    _colorTable = colorTable;

    updateImage();
}

void Chunk2DView::setData(Chunk2D<float> data)
{
    _data = std::move(data);

    if(_window.first == 0 && _window.second == 0) // still default values -> window min/max
        setWindowMinMax();
    else
        updateImage(); // keep previous window
}

void Chunk2DView::plot(Chunk2D<float> data, QPair<double, double> windowing, double zoom)
{
    auto viewer = new Chunk2DView;

    viewer->setWindow(windowing.first, windowing.second);
    viewer->setZoom(zoom);

    viewer->setData(std::move(data));
    viewer->autoResize();
    viewer->setAutoMouseWindowScaling();
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    viewer->show();
}

QPair<double, double> Chunk2DView::windowFromTo() const
{
    return _window;
}

QPair<double, double> Chunk2DView::windowCenterWidth() const
{
    const auto width = _window.second - _window.first;
    const auto center = _window.first + width / 2.0;

    return qMakePair(center, width);
}

void Chunk2DView::setMouseWindowingScaling(double centerScale, double widthScale)
{
    _mouseWindowingScaling.first  = centerScale;
    _mouseWindowingScaling.second = widthScale;
}

void Chunk2DView::setWheelZoomScaling(double scaling)
{
    _wheelZoomScaling = scaling;
}

void Chunk2DView::autoResize()
{
    static const auto maxSize = QSize(1000, 800);
    static const auto margins = QSize(10, 10);

    const auto imgSize = QSize(_data.width(), _data.height()) + margins;

    resize(imgSize.boundedTo(maxSize));
}

void Chunk2DView::setWindowMinMax()
{
    const auto dataMin = static_cast<double>(_data.min());
    const auto dataMax = static_cast<double>(_data.max());

    setWindow(dataMin, dataMax);
}

void Chunk2DView::setZoom(double zoom)
{
    if(zoom < 0.1)
    {
        qWarning() << "Zoom factor too small. It will be ignored.";
        return;
    }

    _zoom = zoom;

    emit zoomChanged(zoom);

    updateImage();
}

void Chunk2DView::setWindow(double from, double to)
{
    if(from > to)
    {
        qWarning() << "Windowing start must not be larger that its end.";
        return;
    }

    _window.first  = from;
    _window.second = to;

    emit windowingChanged(from, to);

    updateImage();
}

void Chunk2DView::setWindowCenterWidth(double center, double width)
{
    const auto from  = center - width / 2.0;
    const auto to = center + width / 2.0;

    setWindow(from, to);
}

void Chunk2DView::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons() == Qt::LeftButton)
    {
        const auto dragVector = event->pos() - _mouseDragStart;

        auto centerAdjust = dragVector.y() * _mouseWindowingScaling.first;
        auto widthAdjust = dragVector.x() * _mouseWindowingScaling.second;

        setWindowCenterWidth(_windowDragStartValue.first + centerAdjust,
                             _windowDragStartValue.second + widthAdjust);

        qInfo() << windowCenterWidth();
    }

    QWidget::mousePressEvent(event);
}

void Chunk2DView::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        _mouseDragStart = event->pos();
        _windowDragStartValue = windowCenterWidth();
    }

    QWidget::mousePressEvent(event);
}

void Chunk2DView::wheelEvent(QWheelEvent* event)
{
    if(event->modifiers() == Qt::CTRL)
    {
        const QPoint numDegrees = event->angleDelta() / 120.0; // in steps of 15 deg
        auto zoomAdjust = numDegrees.y() * _wheelZoomScaling;
        setZoom(_zoom + zoomAdjust);
    }

    QWidget::wheelEvent(event);
}

void Chunk2DView::setGrayscaleColorTable()
{
    _colorTable = QVector<QRgb>(256);

    for(int i = 0; i <= 255; ++i)
        _colorTable[i] = qRgb(i,i,i);
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
    ui->_L_image->setPixmap(pixmap);
}


} // namespace gui
} // namespace CTL
