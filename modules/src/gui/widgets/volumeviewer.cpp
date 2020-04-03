#include "volumeviewer.h"
#include "ui_volumeviewer.h"

#include "io/ctldatabase.h"

#include <QKeyEvent>

namespace CTL {
namespace gui {

static const QVector<QPair<QString, QPair<double, double>>> WINDOW_PRESETS {
    qMakePair(QStringLiteral("Abdomen"), qMakePair( -140.0,  260.0)),
    qMakePair(QStringLiteral("Angio"),   qMakePair(    0.0,  600.0)),
    qMakePair(QStringLiteral("Bone"),    qMakePair( -450.0, 1050.0)),
    qMakePair(QStringLiteral("Brain"),   qMakePair(    0.0,   80.0)),
    qMakePair(QStringLiteral("Chest"),   qMakePair( -160.0,  240.0)),
    qMakePair(QStringLiteral("Lungs"),   qMakePair(-1150.0,  350.0))
};


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

    ui->_TW_volumeOverview->horizontalHeaderItem(2)->setToolTip(QStringLiteral("Density/Attenuation"));

    ui->_W_dataView->setContrastLinePlotLabels("Position on line", "Attenuation");
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

const CompositeVolume& VolumeViewer::data() const { return _compData; }

Chunk2DView* VolumeViewer::dataView() const { return ui->_W_dataView; }

void VolumeViewer::setWindowPresets(QPair<QString, QPair<double, double> > preset1,
                                    QPair<QString, QPair<double, double> > preset2)
{
    ui->_W_windowing->setPresets(preset1, preset2);
}

void VolumeViewer::setWindowPresets(WindowPreset preset1, WindowPreset preset2)
{
    setWindowPresets(WINDOW_PRESETS.at(int(preset1)), WINDOW_PRESETS.at(int(preset2)));
}

void VolumeViewer::setWindowPresetsInMu(WindowPreset preset1, WindowPreset preset2,
                                        float referenceEnergy)
{
    auto HUtoMu =  [ referenceEnergy ] (const QPair<double, double>& windowInHU)
    {
        const auto muWater = attenuationModel(database::Composite::Water)->valueAt(referenceEnergy);
        return qMakePair((windowInHU.first / 1000.0 * muWater) + muWater,
                         (windowInHU.second / 1000.0 * muWater) + muWater);
    };

    const auto preset1HU = WINDOW_PRESETS.at(int(preset1));
    const auto preset2HU = WINDOW_PRESETS.at(int(preset2));

    setWindowPresets(qMakePair(preset1HU.first, HUtoMu(preset1HU.second)),
                     qMakePair(preset2HU.first, HUtoMu(preset2HU.second)));
}

void VolumeViewer::autoResize()
{
    static const auto minimumSize = QSize(800, 400);
    static const auto margin = QSize(300, 100);
    ui->_W_dataView->autoResize();
    resize((ui->_W_dataView->size() + margin).expandedTo(minimumSize));
}

void VolumeViewer::hideCompositeOverview(bool hide)
{
    ui->_TW_volumeOverview->setVisible(!hide);
}

void VolumeViewer::setAutoMouseWindowScaling()
{
    ui->_W_dataView->setAutoMouseWindowScaling();
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

void VolumeViewer::showSubvolume(int subvolume)
{
    ui->_TW_volumeOverview->selectRow(subvolume);
}

void VolumeViewer::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_K)
    {
        ui->_W_dataView->showContrastLinePlot();
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
    constexpr int MAX_NAME_WIDTH = 150;
    constexpr int MAX_TOTAL_WIDTH = 300;

    ui->_TW_volumeOverview->clearContents();
    ui->_TW_volumeOverview->setRowCount(_compData.nbSubVolumes());
    uint row = 0;
    for(const auto& subvol : _compData.data())
    {
        auto nameItem = new QTableWidgetItem(subvol->materialName());
        auto dimItem  = new QTableWidgetItem(QString::fromStdString(subvol->dimensions().info()));
        auto densAttItem = new QTableWidgetItem(subvol->isDensityVolume() ? QStringLiteral("D")
                                                                          : QStringLiteral("A"));
        ui->_TW_volumeOverview->setItem(row, 0, nameItem);
        ui->_TW_volumeOverview->setItem(row, 1, dimItem);
        ui->_TW_volumeOverview->setItem(row, 2, densAttItem);
        ++row;
    }

    ui->_TW_volumeOverview->resizeColumnsToContents();

    // adjust width
    ui->_TW_volumeOverview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    const auto reqWidth = ui->_TW_volumeOverview->horizontalHeader()->length() +
                          ui->_TW_volumeOverview->verticalHeader()->width() +
                          ui->_TW_volumeOverview->frameWidth()*2;
    ui->_TW_volumeOverview->setFixedWidth(std::min(reqWidth, MAX_TOTAL_WIDTH));
    if(ui->_TW_volumeOverview->columnWidth(0) > MAX_NAME_WIDTH)
        ui->_TW_volumeOverview->setColumnWidth(0, MAX_NAME_WIDTH);
    ui->_TW_volumeOverview->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
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
