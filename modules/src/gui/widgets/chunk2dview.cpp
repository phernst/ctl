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
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    viewer->show();
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
    auto dataMin = static_cast<double>(_data.min());
    auto dataMax = static_cast<double>(_data.max());

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

    updateImage();
}

void Chunk2DView::setWindow(double from, double to)
{
    _window.first  = from;
    _window.second = to;

    updateImage();
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
    ui->_L_image->setPixmap(pixmap);
}


} // namespace gui
} // namespace CTL
