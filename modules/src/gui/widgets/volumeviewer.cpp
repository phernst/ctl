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

/*!
 * Creates a VolumeViewer object with \a parent as a parent widget. Note that you need to call
 * show() to display the window.
 *
 * The static method plot() can be used as a convenience alternative for quick visualization.
 */
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

/*!
 * Creates a VolumeViewer with parent widget \a parent and sets its data to \a volume. Note that
 * you need to call show() to display the window.
 *
 * The static method plot() can be used as a convenience alternative for quick visualization.
 */
VolumeViewer::VolumeViewer(CompositeVolume volume, QWidget *parent)
    : VolumeViewer(parent)
{
    setData(std::move(volume));
}

/*!
 * Deletes the object.
 */
VolumeViewer::~VolumeViewer()
{
    delete ui;
}

/*!
 * Creates a VolumeViewer to visualize \a data and shows the window.
 *
 * Sensitivity of windowing using mouse gestures is adapted automatically to \a data (see
 * setAutoMouseWindowScaling()).
 *
 * The widget will be deleted automatically if the window is closed.
 */
void VolumeViewer::plot(CompositeVolume data)
{
    auto viewer = new VolumeViewer(std::move(data));
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->autoResize();
    viewer->ui->_W_dataView->setAutoMouseWindowScaling();

    viewer->show();
}

/*!
 * Creates a VolumeViewer to visualize \a data and shows the window.
 *
 * Sensitivity of windowing using mouse gestures is adapted automatically to \a data (see
 * setAutoMouseWindowScaling()).
 *
 * The widget will be deleted automatically if the window is closed.
 */
void VolumeViewer::plot(SpectralVolumeData data)
{
    plot(CompositeVolume(std::move(data)));
}

/*!
 * Returns a (constant) reference to the data currently managed by this instance.
 */
const CompositeVolume& VolumeViewer::data() const { return _compData; }

/*!
 * Returns the viewport for displaying the actual slice data. Use this to configure the
 * specific settings of the viewport.
 *
 * \sa Chunk2DView.
 */
Chunk2DView* VolumeViewer::dataView() const { return ui->_W_dataView; }

/*!
 * Sets the visualized data to \a data. Data is copied, so consider moving if it is no longer
 * required.
 *
 * Applies a min/max windowing if no specific windowing has been set (ie. the current window is
 * [0,0]).
 */
void VolumeViewer::setData(SpectralVolumeData data)
{
    setData(CompositeVolume(std::move(data)));
}

/*!
 * Sets the visualized data to \a data. Data is copied, so consider moving if it is no longer
 * required.
 *
 * By default, always selects the first subvolume in \a data for visualization.
 *
 * Applies a min/max windowing if no specific windowing has been set (ie. the current window is
 * [0,0]).
 */
void VolumeViewer::setData(CompositeVolume data)
{
    _compData = std::move(data);

    bool needsAutoWindow = (ui->_W_windowing->windowFromTo() == qMakePair(0.0, 0.0));

    updateVolumeOverview();
    if(_compData.nbSubVolumes())
        ui->_TW_volumeOverview->selectRow(0);

    selectCentralSlice();
    if(needsAutoWindow)
        ui->_W_dataView->setWindowingMinMax();
}

/*!
 * Sets the presets of the two preset buttons in the windowing GUI block to \a preset1 and
 * \a preset2. Presets must contain the text that shall be shown on the button and the pair of
 * values, specifying start and end of the data window, as a QPair.
 *
 * The window range will be shown as a tooltip when hovering the cursor over the corresponding
 * button.
 *
 * Example:
 * \code
 * auto viewer = new VolumeViewer;
 * // ...
 *
 * //                         Button label      Window: ( start,  end  )
 * auto myPreset1 = qMakePair(QString("Soft"), qMakePair(-100.0,  200.0));
 * auto myPreset2 = qMakePair(QString("Bone"), qMakePair(   0.0, 1200.0));
 *
 * viewer->setWindowPresets(myPreset1, myPreset2);
 * viewer->show();
 * \endcode
 */
void VolumeViewer::setWindowPresets(QPair<QString, QPair<double, double> > preset1,
                                    QPair<QString, QPair<double, double> > preset2)
{
    ui->_W_windowing->setPresets(preset1, preset2);
}

/*!
 * Sets the presets of the two preset buttons in the windowing GUI block to \a preset1 and
 * \a preset2. Presets can be chosen from a set of pre-defined window ranges, specified in
 * Hounsfield units (HU) [from, to]:
 * - WindowPreset::Abdomen: [ -140.0,  260.0]
 * - WindowPreset::Angio:   [    0.0,  600.0]
 * - WindowPreset::Bone:    [ -450.0, 1050.0]
 * - WindowPreset::Brain:   [    0.0,   80.0]
 * - WindowPreset::Chest:   [ -160.0,  240.0]
 * - WindowPreset::Lungs:   [-1150.0,  350.0]
 *
 * Example:
 * \code
 * auto viewer = new VolumeViewer;
 * // ...
 *
 * viewer->setWindowPresets(WindowPreset::Abdomen, WindowPreset::Lungs);
 * viewer->show();
 * \endcode
 */
void VolumeViewer::setWindowPresets(WindowPreset preset1, WindowPreset preset2)
{
    setWindowPresets(WINDOW_PRESETS.at(int(preset1)), WINDOW_PRESETS.at(int(preset2)));
}

/*!
 * Sets the presets of the two preset buttons in the windowing GUI block to \a preset1 and
 * \a preset2. Presets can be chosen from a set of pre-defined window ranges (see
 * setWindowPresets(WindowPreset, WindowPreset)). Window ranges from the presets (defined in
 * Hounsfield units) will be converted to attenuation coefficients with respect to the reference
 * energy \a referenceEnergy.
 *
 * Example:
 * \code
 * auto viewer = new VolumeViewer;
 * // ...
 *
 * // set Abdomen and Lung presets for attenuation coefficients with reference energy 75 keV
 * viewer->setWindowPresets(WindowPreset::Abdomen, WindowPreset::Lungs, 75.0f);
 * viewer->show();
 * \endcode
 */
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

/*!
 * Requests an automatic resizing of this widget's window size. The window is tried to fit to the
 * size of the shown data, bounded between a minimum size of 700 x 400 pixels (500 x 400 with
 * hidden composite overview) and a maximum size of 1300 x 900 pixels.
 */
void VolumeViewer::autoResize()
{
    static const auto staticMinSize = QSize(500, 400);
    static const auto staticMargin = QSize(108, 118);

    QSize minimumSize = staticMinSize;
    QSize totalMargin = staticMargin;
    if(!ui->_TW_volumeOverview->isHidden()) // the overview is shown and needs extra space
    {
        totalMargin.rwidth() += (ui->_TW_volumeOverview->width() - 14);
        minimumSize.rwidth() += 200;
    }

    ui->_W_dataView->autoResize();
    resize((ui->_W_dataView->size() + totalMargin).expandedTo(minimumSize));
}

/*!
 * Hides the composite overview table if \a hide = \c true (or makes it visible again if \c false).
 */
void VolumeViewer::hideCompositeOverview(bool hide)
{
    ui->_TW_volumeOverview->setVisible(!hide);
}

/*!
 * Convenience method to set automatically determined values for the sensitvity of windowing using
 * mouse gestures.
 *
 * Same as: \code dataView()->setAutoMouseWindowScaling(); \endcode
 *
 * \sa Chunk2DView::setAutoMouseWindowScaling().
 */
void VolumeViewer::setAutoMouseWindowScaling()
{
    ui->_W_dataView->setAutoMouseWindowScaling();
}

/*!
 * Shows the slice with index \a slice in the currently selected slice direction.
 */
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

/*!
 * Shows the subvolume with index \a subvolume from the currently managed dataset.
 */
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

void VolumeViewer::selectCentralSlice()
{
    if(ui->_RB_directionX->isChecked())
        ui->_VS_slice->setValue(static_cast<int>(std::floor(selectedVolume().nbVoxels().x / 2.0)));
    else if(ui->_RB_directionY->isChecked())
        ui->_VS_slice->setValue(static_cast<int>(std::floor(selectedVolume().nbVoxels().y / 2.0)));
    else if(ui->_RB_directionZ->isChecked())
        ui->_VS_slice->setValue(static_cast<int>(std::floor(selectedVolume().nbVoxels().z / 2.0)));
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
    constexpr int MAX_TOTAL_WIDTH = 306;

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
