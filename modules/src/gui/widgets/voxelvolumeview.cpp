#include "voxelvolumeview.h"
#include "ui_voxelvolumeview.h"

VoxelVolumeView::VoxelVolumeView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VoxelVolumeView),
    _colorTable(256)
{
    ui->setupUi(this);

    setColorTable();

    connect(ui->_W_windowing, SIGNAL(autoWindowingRequested()), SLOT(autoWindowing()));
    connect(ui->_W_windowing, SIGNAL(windowingChanged()), SLOT(updateImage()));
}

VoxelVolumeView::~VoxelVolumeView()
{
    delete ui;
}

void VoxelVolumeView::on_verticalSlider_valueChanged(int value)
{
    ui->_L_slice->setText(QString::number(value));
}

void VoxelVolumeView::updateImage()
{
    QImage image(int(_data.nbVoxels().x),
                 int(_data.nbVoxels().y),
                 QImage::Format_Indexed8);

    image.setColorTable(_colorTable);

    const auto windowing = ui->_W_windowing->windowFromTo();

    const auto minGrayValue = static_cast<float>(windowing.first);
    const auto maxGrayValue = static_cast<float>(windowing.second);
    const auto grayScale = 255.0f / float(maxGrayValue - minGrayValue);
    const auto X = image.width();
    const auto Y = image.height();
    const auto z = uint(ui->verticalSlider->value());

    float grayValue;

    if(minGrayValue == maxGrayValue)
    {
        for(int y = 0; y < Y; ++y)
            for(int x = 0; x < X; ++x)
            {
                grayValue = (_data(x,y,z) > minGrayValue) ? 255.0f : 0.0f;
                image.setPixel(x,y,int(grayValue));
            }
    }
    else
    {
        for(int y = 0; y < Y; ++y)
            for(int x = 0; x < X; ++x)
            {
                grayValue = (_data(x,y,z) - minGrayValue) * grayScale;
                image.setPixel(x,y,qMax(qMin(int(grayValue + .5f),255),0));
            }
    }
    auto pixmap = QPixmap::fromImage(image).scaledToHeight(image.height() * ui->_SB_zoom->value());
    ui->_L_image->setPixmap(pixmap);
}

void VoxelVolumeView::updateSliderRange()
{
    ui->verticalSlider->setMaximum(_data.nbVoxels().z - 1);
}

void VoxelVolumeView::autoWindowing()
{
    auto dataMin = static_cast<double>(_data.min());
    auto dataMax = static_cast<double>(_data.max());

    ui->_W_windowing->setWindowFromTo(qMakePair(dataMin, dataMax));
}

void VoxelVolumeView::setColorTable()
{
    for(int i = 0; i <= 255; ++i)
        _colorTable[i] = qRgb(i,i,i);
}


