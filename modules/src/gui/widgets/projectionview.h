#ifndef PROJECTIONVIEW_H
#define PROJECTIONVIEW_H

#include <QWidget>
#include "img/projectiondata.h"

namespace Ui {
class ProjectionView;
}

class ProjectionView : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectionView(QWidget *parent = 0);
    ~ProjectionView();

    void setData(const CTL::ProjectionData& projections);

private:
    Ui::ProjectionView *ui;

    CTL::ProjectionData _data = CTL::ProjectionData(0,0,0);
    QVector<QRgb> _colorTable;

    void setColorTable();

private slots:
    void on_verticalSlider_valueChanged(int value);
    void updateImage();
    void updateSliderRange();
    void autoWindowing();

};
#endif // PROJECTIONVIEW_H
