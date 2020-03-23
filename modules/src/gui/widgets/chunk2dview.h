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

public slots:
    void autoResize();
    void setWindowMinMax();
    void setZoom(double zoom);
    void setWindow(double from, double to);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    Ui::Chunk2DView *ui;

    Chunk2D<float> _data = Chunk2D<float>(0,0);
    QVector<QRgb> _colorTable;
    QPair<double, double> _window;

    double _zoom = 1.0;

    void setGrayscaleColorTable();
    void updateImage();
};

} // namespace gui
} // namespace CTL

#endif // CTL_CHUNK2DVIEW_H
