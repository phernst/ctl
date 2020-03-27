#include "volumeviewer.h"
#include "ui_volumeviewer.h"

#ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
    #include "gui/widgets/lineseriesview.h"
#endif

#include <QKeyEvent>

namespace CTL {
namespace gui {

VolumeViewer::VolumeViewer(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::VolumeViewer)
{
    ui->setupUi(this);

    connect(ui->_VS_slice, &QSlider::valueChanged, this, &VolumeViewer::showSlice);
    // connections for windowing
    connect(ui->_W_windowing, &WindowingWidget::windowingChanged, this, &VolumeViewer::windowingUpdate);
    connect(ui->_W_windowing, &WindowingWidget::autoWindowingRequested, ui->_W_dataView, &Chunk2DView::setWindowingMinMax);
    connect(ui->_W_dataView, &Chunk2DView::windowingChanged, ui->_W_windowing, &WindowingWidget::setWindowDataSilent);
    // connections for zoom
    connect(ui->_W_zoomControl, &ZoomControlWidget::zoomRequested, ui->_W_dataView, &Chunk2DView::setZoom);
    connect(ui->_W_dataView, &Chunk2DView::zoomChanged, ui->_W_zoomControl, &ZoomControlWidget::setZoomValueSilent);
    // connections for live pixel info
    connect(ui->_W_dataView, &Chunk2DView::pixelInfoUnderCursor, this, &VolumeViewer::updatePixelInfo);

    // connections for slice direction
    connect(ui->_RB_directionX, &QRadioButton::toggled, this, &VolumeViewer::sliceDirectionChanged);
    connect(ui->_RB_directionY, &QRadioButton::toggled, this, &VolumeViewer::sliceDirectionChanged);
    connect(ui->_RB_directionZ, &QRadioButton::toggled, this, &VolumeViewer::sliceDirectionChanged);

    // connections for subvolume selection
    connect(ui->_TW_volumeOverview, &QTableWidget::itemSelectionChanged, this, &VolumeViewer::volumeSelectionChanged);

    ui->_W_dataView->setLivePixelDataEnabled(true);

    resize(900, 600);
    setWindowTitle("Volume Viewer");
}

VolumeViewer::VolumeViewer(CompositeVolume volume, QWidget *parent)
    : VolumeViewer(parent)
{
    setData(std::move(volume));
}

VolumeViewer::~VolumeViewer()
{
    delete ui;
}

void VolumeViewer::setData(SpectralVolumeData data)
{
    _compData = CompositeVolume(std::move(data));

    updateVolumeOverview();
    ui->_TW_volumeOverview->selectRow(0);
}

void VolumeViewer::setData(CompositeVolume data)
{
    _compData = std::move(data);

    updateVolumeOverview();
    if(_compData.nbSubVolumes())
        ui->_TW_volumeOverview->selectRow(0);
}

void VolumeViewer::plot(CompositeVolume data)
{
    auto viewer = new VolumeViewer(std::move(data));
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->autoResize();
    viewer->ui->_W_dataView->setAutoMouseWindowScaling();

    viewer->show();
}

void VolumeViewer::plot(SpectralVolumeData data)
{
    plot(CompositeVolume(std::move(data)));
}

void VolumeViewer::autoResize()
{
    static const auto minimumSize = QSize(850, 400);
    static const auto margin = QSize(300, 100);
    ui->_W_dataView->autoResize();
    resize((ui->_W_dataView->size() + margin).expandedTo(minimumSize));
}

void VolumeViewer::showSlice(int slice)
{
    ui->_L_slice->setNum(slice);
    if(ui->_RB_directionX->isChecked())
        ui->_W_dataView->setData(selectedVolume().sliceX(static_cast<uint>(slice)));
    else if(ui->_RB_directionY->isChecked())
        ui->_W_dataView->setData(selectedVolume().sliceY(static_cast<uint>(slice)));
    else if(ui->_RB_directionZ->isChecked())
        ui->_W_dataView->setData(selectedVolume().sliceZ(static_cast<uint>(slice)));

}

void VolumeViewer::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_K)
    {
    #ifdef GUI_WIDGETS_CHARTS_MODULE_AVAILABLE
        LineSeriesView::plot(ui->_W_dataView->contrastLine(), "Distance on line", "Attenuation");
        event->accept();
    #endif
    }
    else if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        ui->_W_dataView->saveDialog();
        event->accept();
    }

    QWidget::keyPressEvent(event);
}

const SpectralVolumeData& VolumeViewer::selectedVolume() const
{
    return _compData.subVolume(ui->_TW_volumeOverview->selectedItems().first()->row());
}

void VolumeViewer::sliceDirectionChanged()
{
    updateSliderRange();
    showSlice(ui->_VS_slice->value());
}

void VolumeViewer::volumeSelectionChanged()
{
    updateSliderRange();
    showSlice(ui->_VS_slice->value());
}

void VolumeViewer::updateVolumeOverview()
{
    ui->_TW_volumeOverview->clearContents();

    ui->_TW_volumeOverview->setRowCount(_compData.nbSubVolumes());
    uint row = 0;
    for(const auto& subvol : _compData.data())
    {
        auto nameItem = new QTableWidgetItem(subvol->materialName());
        auto dimItem  = new QTableWidgetItem(QString::fromStdString(subvol->dimensions().info()));
        ui->_TW_volumeOverview->setItem(row, 0, nameItem);
        ui->_TW_volumeOverview->setItem(row, 1, dimItem);
        ++row;
    }
}

void VolumeViewer::updateSliderRange()
{
    if(ui->_RB_directionX->isChecked())
        ui->_VS_slice->setMaximum(selectedVolume().dimensions().x - 1);
    else if(ui->_RB_directionY->isChecked())
        ui->_VS_slice->setMaximum(selectedVolume().dimensions().y - 1);
    else if(ui->_RB_directionZ->isChecked())
        ui->_VS_slice->setMaximum(selectedVolume().dimensions().z - 1);
}

void VolumeViewer::updatePixelInfo(int x, int y, float value)
{
    QString info = QStringLiteral("(") + QString::number(x) + QStringLiteral(" , ")
                                       + QString::number(y) + QStringLiteral("): ")
                                       + QString::number(value);
    ui->_L_pixelInfo->setText(info);
}

void VolumeViewer::windowingUpdate()
{
    auto newWindowing = ui->_W_windowing->windowFromTo();
    ui->_W_dataView->setWindowing(newWindowing.first, newWindowing.second);
}

} // namespace gui
} // namespace CTL
