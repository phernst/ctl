#include "projectionview.h"
#include "ui_projectionview.h"

#include <QDebug>

ProjectionView::ProjectionView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionView),
    _colorTable(256)
{
    ui->setupUi(this);

    setColorTable();

    connect(ui->_W_windowing, SIGNAL(autoWindowingRequested()), SLOT(autoWindowing()));
    connect(ui->_W_windowing, SIGNAL(windowingChanged()), SLOT(updateImage()));
}

ProjectionView::~ProjectionView()
{
    delete ui;
}

void ProjectionView::setData(const CTL::ProjectionData &projections)
{
    _data = projections;

    updateSliderRange();
    updateImage();
}

void ProjectionView::setModuleLayout(const CTL::ModuleLayout &layout)
{
    _modLayout = layout;
    updateImage();
}

void ProjectionView::on_verticalSlider_valueChanged(int value)
{
    ui->_L_slice->setText(QString::number(value));
}

void ProjectionView::updateImage()
{
    if(_data.nbViews() == 0)
        return;

    const auto z = uint(ui->verticalSlider->value());
    auto projection = _data.view(z).combined(_modLayout);

    QImage image(int(projection.dimensions().width),
                 int(projection.dimensions().height),
                 QImage::Format_Indexed8);

    image.setColorTable(_colorTable);

    const auto windowing = ui->_W_windowing->windowFromTo();
    const auto minGrayValue = static_cast<float>(windowing.first);
    const auto maxGrayValue = static_cast<float>(windowing.second);
    const auto grayScale = 255.0f / float(maxGrayValue - minGrayValue);
    const auto X = image.width();
    const auto Y = image.height();

    float grayValue;
    for(int y = 0; y < Y; ++y)
        for(int x = 0; x < X; ++x)
        {
            grayValue = (projection(x,y) - minGrayValue) * grayScale;
            image.setPixel(x,y,qMax(qMin(int(grayValue + .5f),255),0));
        }

    auto pixmap = QPixmap::fromImage(image).scaledToHeight(image.height() * ui->_SB_zoom->value());
    ui->_L_image->setPixmap(pixmap);
}

void ProjectionView::updateSliderRange()
{
    ui->verticalSlider->setMaximum(_data.dimensions().nbViews- 1);
}

void ProjectionView::autoWindowing()
{
    auto dataMin = static_cast<double>(_data.min());
    auto dataMax = static_cast<double>(_data.max());

    ui->_W_windowing->setWindowFromTo(qMakePair(dataMin, dataMax));
}

void ProjectionView::setColorTable()
{
    for(int i = 0; i <= 255; ++i)
        _colorTable[i] = qRgb(i,i,i);
}

