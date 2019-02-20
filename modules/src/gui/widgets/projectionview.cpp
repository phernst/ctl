#include "projectionview.h"
#include "ui_projectionview.h"

#include <cmath>

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

    if(ui->_W_windowing->windowFromTo().first == 0 &&
       ui->_W_windowing->windowFromTo().second == 0)
        autoWindowing();
    else
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
    const auto& projection = _data.dimensions().nbModules > 1
                           ? _data.view(z).combined(_modLayout)
                           : _data.view(z).module(0);
    const auto imgWidth = int(projection.dimensions().width);
    const auto imgHeight = int(projection.dimensions().height);

    QImage image(imgWidth, imgHeight, QImage::Format_Indexed8);
    image.setColorTable(_colorTable);

    const auto windowing = ui->_W_windowing->windowFromTo();
    const auto minGrayValue = static_cast<float>(windowing.first);
    const auto maxGrayValue = static_cast<float>(windowing.second);
    const auto grayScale = 255.0f / float(maxGrayValue - minGrayValue);
    const auto offset = - minGrayValue * grayScale + 0.5f; // 0.5 for rounding

    auto projIt = projection.data().begin();
    for(int y = 0; y < imgHeight; ++y)
    {
        auto linePtr = image.scanLine(y);
        for(int x = 0; x < imgWidth; ++x)
        {
            *linePtr = static_cast<uchar>(qMax(qMin( // clamp to interval [0, 255]
                std::fma(*projIt, grayScale, offset), 255.f), 0.f)); // projVal * grayScale + offset
            ++linePtr;
            ++projIt;
        }
    }

    auto pixmap = QPixmap::fromImage(image).scaledToHeight(imgHeight * ui->_SB_zoom->value());
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

