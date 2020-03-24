#ifndef CTL_CHUNK2DVIEW_H
#define CTL_CHUNK2DVIEW_H

#include <QScrollArea>

#include "img/chunk2d.h"

class QLabel;

namespace CTL {
namespace gui {

class Chunk2DView : public QScrollArea
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
    const QPixmap* pixmap() const;
    QPair<double, double> windowingFromTo() const;
    QPair<double, double> windowingCenterWidth() const;
    double zoom() const;

    // setter methods
    void setColorTable(const QVector<QRgb>& colorTable);
    void setData(Chunk2D<float> data);
    void setMouseWindowingScaling(double centerScale, double widthScale);
    void setWheelZoomPerTurn(double zoomPerTurn);

public slots:
    void autoResize();
    void setLivePixelDataEnabled(bool enabled);
    void setWindowing(double from, double to);
    void setWindowingCenterWidth(double center, double width);
    void setWindowingMinMax();
    void setZoom(double zoom);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

signals:
    void pixelInfoUnderCursor(int x, int y, float value);
    void windowingChanged(double from, double to);
    void zoomChanged(double zoom);

private:
    QLabel* _imageLabel;

    Chunk2D<float> _data = Chunk2D<float>(0,0);
    QVector<QRgb> _colorTable;
    QPair<double, double> _window;
    double _zoom = 1.0;

    // event handling
    QPoint _mouseDragStart;
    QPair<double, double> _windowDragStartValue;
    QPair<double, double> _mouseWindowingScaling = {1.0, 1.0};
    double _wheelZoomPerTurn = 0.25; // i.e 0.25 zoom per 15.0 deg;

    void setGrayscaleColorTable();
    void setAutoMouseWindowScaling();
    void updateImage();
    QPoint pixelIdxFromPos(const QPoint& pos);
};

} // namespace gui
} // namespace CTL

#endif // CTL_CHUNK2DVIEW_H
