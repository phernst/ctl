#ifndef CTL_CHUNK2DVIEW_H
#define CTL_CHUNK2DVIEW_H

#include <QWidget>

#include "img/chunk2d.h"

namespace Ui {
class Chunk2DView;
}

namespace CTL {
namespace gui {

class Chunk2DView : public QWidget
{
    Q_OBJECT

public:
    explicit Chunk2DView(QWidget* parent = nullptr);
    Chunk2DView(Chunk2D<float> data, QWidget* parent = nullptr);
    ~Chunk2DView();

    void setColorTable(const QVector<QRgb>& colorTable);
    void setData(Chunk2D<float> data);

    static void plot(Chunk2D<float> data,
                     QPair<double,double> windowing = qMakePair(0.0, 0.0), double zoom = 1.0);

    QPair<double, double> windowFromTo() const;
    QPair<double, double> windowCenterWidth() const;

    void setMouseWindowingScaling(double centerScale, double widthScale);
    void setWheelZoomScaling(double scaling);

public slots:
    void autoResize();
    void setWindowMinMax();
    void setZoom(double zoom);
    void setWindow(double from, double to);
    void setWindowCenterWidth(double center, double width);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

signals:
    void windowingChanged(double from, double to);
    void zoomChanged(double zoom);

private:
    Ui::Chunk2DView *ui;

    Chunk2D<float> _data = Chunk2D<float>(0,0);
    QVector<QRgb> _colorTable;
    QPair<double, double> _window;
    QPoint _mouseDragStart;
    QPair<double, double> _windowDragStartValue;

    QPair<double, double> _mouseWindowingScaling = {1.0, 1.0};
    double _wheelZoomScaling = 0.25; // i.e 0.25 zoom per 15.0 deg;

    double _zoom = 1.0;

    void setGrayscaleColorTable();
    void setAutoMouseWindowScaling();
    void updateImage();
};

} // namespace gui
} // namespace CTL

#endif // CTL_CHUNK2DVIEW_H
