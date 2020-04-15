#include "projectionviewer.h"
#include "ui_projectionviewer.h"

#include <QKeyEvent>
#include <QDebug>

namespace CTL {
namespace gui {

/*!
 * Creates a ProjectionViewer object with \a parent as a parent widget. Note that you need to call
 * show() to display the window.
 *
 * The static method plot() can be used as a convenience alternative for quick visualization.
 */
ProjectionViewer::ProjectionViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionViewer)
{
    ui->setupUi(this);

    connect(ui->_VS_projection, &QSlider::valueChanged, this, &ProjectionViewer::showView);
    connect(ui->_W_dataView, &Chunk2DView::viewChangeRequested, this, &ProjectionViewer::changeView);
    // connections for windowing
    connect(ui->_W_windowing, &WindowingWidget::windowingChanged, this, &ProjectionViewer::windowingUpdate);
    connect(ui->_W_windowing, &WindowingWidget::autoWindowingRequested, ui->_W_dataView, &Chunk2DView::setWindowingMinMax);
    connect(ui->_W_dataView, &Chunk2DView::windowingChanged, ui->_W_windowing, &WindowingWidget::setWindowDataSilent);
    // connections for zoom
    connect(ui->_W_zoomControl, &ZoomControlWidget::zoomRequested, ui->_W_dataView, &Chunk2DView::setZoom);
    connect(ui->_W_dataView, &Chunk2DView::zoomChanged, ui->_W_zoomControl, &ZoomControlWidget::setZoomValueSilent);
    // connections for live pixel info
    connect(ui->_W_dataView, &Chunk2DView::pixelInfoUnderCursor, this, &ProjectionViewer::updatePixelInfo);

    ui->_W_dataView->setLivePixelDataEnabled(true);
    ui->_W_dataView->setContrastLinePlotLabels("Position on line", "Extinction");

    setWindowPresets(qMakePair(QStringLiteral("Narrow"), qMakePair(0.0,  2.0)),
                     qMakePair(QStringLiteral("Wide"),   qMakePair(0.0, 10.0)));

    resize(1000, 800);
    setWindowTitle("Projection Viewer");
}

/*!
 * Creates a ProjectionViewer with parent widget \a parent and sets its data to \a projections. Note
 * that you need to call show() to display the window.
 *
 * The static method plot() can be used as a convenience alternative for quick visualization.
 */
ProjectionViewer::ProjectionViewer(ProjectionData projections, QWidget* parent)
    : ProjectionViewer(parent)
{
    setData(projections);
}

/*!
 * Deletes the object.
 */
ProjectionViewer::~ProjectionViewer()
{
    delete ui;
}

/*!
 * Creates a ProjectionViewer for \a data and shows the window. If a specific ModuleLayout is passed
 * by \a layout, the data will be combined according to this layout for the purpose of displaying.
 * Otherwise, all modules in \a data are assumed to be arranged in a line next to each other in a
 * horizontal manner.
 *
 * Sensitivity of windowing using mouse gestures is adapted automatically to \a data (see
 * setAutoMouseWindowScaling()).
 *
 * The widget will be deleted automatically if the window is closed.
 */
void ProjectionViewer::plot(ProjectionData projections, const ModuleLayout& layout)
{
    auto viewer = new ProjectionViewer(projections);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setModuleLayout(layout);
    viewer->autoResize();
    viewer->ui->_W_dataView->setAutoMouseWindowScaling();

    viewer->show();
}

/*!
 * Returns a (constant) reference to the data currently managed by this instance.
 */
const ProjectionData& ProjectionViewer::data() const { return _data; }

/*!
 * Returns the viewport for displaying the actual projection data. Use this to configure the
 * specific settings of the viewport.
 *
 * \sa Chunk2DView.
 */
Chunk2DView* ProjectionViewer::dataView() const { return ui->_W_dataView; }

/*!
 * Sets the visualized data to \a projections. Data is copied, so consider moving if it is no
 * longer required.
 *
 * Applies a min/max windowing if no specific windowing has been set (ie. the current window is
 * [0,0]).
 */
void ProjectionViewer::setData(ProjectionData projections)
{
    _data = std::move(projections);

    updateSliderRange();

    if(_data.nbViews())
        showView(0);
}

/*!
 * Sets the module layout used to combine data from individual detector modules to \a layout. Data
 * can only be shown on the basis of one combined block of data (Chunk2D).
 *
 * Calling this method after data has been set will update the visualization with respect to the new
 * layout. Note that the passed layout needs to be compatible with the projection data.
 */
void ProjectionViewer::setModuleLayout(const ModuleLayout& layout)
{
    _modLayout = layout;

    if(_data.nbViews() > 0)
        showView(currentView()); // recompute data due to changed layout (if data is available)
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
 * auto viewer = new ProjectionViewer;
 * // ...
 *
 * //                         Button label      Window: ( start ,  end   )
 * auto myPreset1 = qMakePair(QString("High"), qMakePair(1000.0, 100000.0));
 * auto myPreset2 = qMakePair(QString("Low"), qMakePair(-10.0, 10.0));
 *
 * viewer->setWindowPresets(myPreset1, myPreset2);
 * viewer->show();
 * \endcode
 */
void ProjectionViewer::setWindowPresets(QPair<QString, QPair<double, double> > preset1,
                                        QPair<QString, QPair<double, double> > preset2)
{
    ui->_W_windowing->setPresets(preset1, preset2);
}

/*!
 * Returns the index of the view currently shown in the viewer.
 */
int ProjectionViewer::currentView() const
{
    return ui->_VS_projection->value();
}

/*!
 * Requests an automatic resizing of this widget's window size. The window is tried to fit to the
 * size of the shown data, bounded to a maximum size of 1090 x 915 pixels.
 */
void ProjectionViewer::autoResize()
{
    static const auto margin = QSize(90, 118);
    ui->_W_dataView->autoResize();
    resize(ui->_W_dataView->size() + margin);
}

/*!
 * Convenience method to set automatically determined values for the sensitvity of windowing using
 * mouse gestures.
 *
 * Same as: \code dataView()->setAutoMouseWindowScaling(); \endcode
 *
 * \sa Chunk2DView::setAutoMouseWindowScaling().
 */
void ProjectionViewer::setAutoMouseWindowScaling()
{
    ui->_W_dataView->setAutoMouseWindowScaling();
}

/*!
 * Shows view number \a view from the currently managed data; \a view must be a valid index, ie.
 * 0 <= \a view < data().nbViews().
 */
void ProjectionViewer::showView(int view)
{
    ui->_L_view->setText(QString::number(view));
    ui->_W_dataView->setData(_data.view(view).combined(_modLayout));
}

void ProjectionViewer::keyPressEvent(QKeyEvent* event)
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

void ProjectionViewer::changeView(int requestedChange)
{
    const auto curViewIdx = ui->_VS_projection->value();
    ui->_VS_projection->setValue(curViewIdx + requestedChange);
}

void ProjectionViewer::updateSliderRange()
{
    ui->_VS_projection->setMaximum(_data.dimensions().nbViews - 1);
}

void ProjectionViewer::updatePixelInfo(int x, int y, float value)
{
    ui->_L_pixelInfo->setText("(" + QString::number(x) + " , " + QString::number(y) + "): "
                              + QString::number(value));
}

void ProjectionViewer::windowingUpdate()
{
    auto newWindowing = ui->_W_windowing->windowFromTo();
    ui->_W_dataView->setWindowing(newWindowing.first, newWindowing.second);
}

} // namespace gui
} // namespace CTL
