#include "chartviewbase.h"
#include <QLineSeries>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QFileDialog>

using namespace QtCharts;

namespace CTL {
namespace gui {

ChartViewBase::ChartViewBase(QWidget* parent)
    : QChartView(parent)
    , _chart(new QChart)
    , _dataSeries(new QLineSeries)
    , _dataSeriesLog(new QLineSeries)
{
    setChart(_chart);
    _chart->legend()->hide();

    setRubberBand(QChartView::RectangleRubberBand);
}

QImage ChartViewBase::image(const QSize& renderSize)
{
    QSize imgSize = renderSize.isValid() ? renderSize : size();

    QImage ret(imgSize, QImage::Format_ARGB32);
    QPainter painter(&ret);

    render(&painter);

    return ret;
}

void ChartViewBase::autoRange()
{
    auto compareX = [] (const QPointF& a, const QPointF& b) { return a.x()<b.x(); };
    auto compareY = [] (const QPointF& a, const QPointF& b) { return a.y()<b.y(); };

    const auto dataPts = yAxisIsLinear() ? _dataSeries->pointsVector()
                                         : _dataSeriesLog->pointsVector();

    const auto minMaxX = std::minmax_element(dataPts.cbegin(), dataPts.cend(), compareX);
    const auto minMaxY = std::minmax_element(dataPts.cbegin(), dataPts.cend(), compareY);

    auto xRange = qMakePair(minMaxX.first->x(), minMaxX.second->x());
    auto yRange = qMakePair(minMaxY.first->y(), minMaxY.second->y());

    // avoid zero-ranges
    if(qFuzzyCompare(xRange.first, xRange.second))
    {
        xRange.first  -= 1.0;
        xRange.second += 1.0;
    }
    if(qFuzzyCompare(yRange.first, yRange.second))
    {
        yRange.first  -= 1.0;
        yRange.second += 1.0;
    }

    setRangeX(xRange.first, xRange.second);
    setRangeY(yRange.first, yRange.second);
}

/*!
 * Saves the image currently shown by this instance to the file \a fileName.
 *
 * The file type must be an image file type supported by Qt and will be determined automatically
 * from the ending of \a fileName. If no file type ending is found, or it is incompatible, a PNG
 * file is created.
 *
 * Same as: \code image().save(fileName) \endcode
 */
bool ChartViewBase::save(const QString& fileName)
{
    return image().save(fileName);
}

/*!
 * Opens a save file dialog to get the file name used to save the currently shown image to a file.
 *
 * \sa save().
 */
void ChartViewBase::saveDialog()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save plot", "", "Images (*.png *.jpg *.bmp)");
    if(fn.isEmpty())
        return;

    save(fn);
}

void ChartViewBase::setLabelX(const QString &label)
{
    myAxisX(_plottableSeries)->setTitleText(label);
    myAxisX(_plottableSeriesLog)->setTitleText(label);
}

void ChartViewBase::setLabelY(const QString &label)
{
    myAxisY(_plottableSeries)->setTitleText(label);
    myAxisY(_plottableSeriesLog)->setTitleText(label);
}

void ChartViewBase::setLogAxisY(bool enabled)
{
    enabled ? switchToLogAxisY() : switchToLinAxisY();
}

void ChartViewBase::setRangeX(double from, double to)
{
    if(yAxisIsLinear())
    {
        myAxisX(_plottableSeries)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(myAxisX(_plottableSeries))->applyNiceNumbers();
    }
    else
    {
        myAxisX(_plottableSeriesLog)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(myAxisX(_plottableSeriesLog))->applyNiceNumbers();
    }
}

/*!
 * Sets the range of the *y*-axis to [from, to].
 *
 * Note that the upper end (ie. \a to) of the range is adjusted if "*y*-axis over ranging" is enabled
 * (see setOverRangeY()).
 */
void ChartViewBase::setRangeY(double from, double to)
{
    if(_overRangeY)
    {
        const auto width = to - from;
        to += 0.01 * width;
    }

    if(yAxisIsLinear())
        myAxisY(_plottableSeries)->setRange(from, to);
    else
        myAxisY(_plottableSeriesLog)->setRange(from, to);
}

/*!
 * Sets the usage of the "Nice X mode" to \a enabled. If activated, this uses
 * QtCharts::QValueAxis::applyNiceNumbers() to adjust the range of the *x*-axis.
 */
void ChartViewBase::setUseNiceX(bool enabled) { _useNiceX = enabled; }

/*!
 * Switches between the linear and logarithmic mode of the *y*-axis.
 */
void ChartViewBase::toggleLinLogY()
{
    if(yAxisIsLinear())
        switchToLogAxisY();
    else
        switchToLinAxisY();
}

void ChartViewBase::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
        autoRange();

    event->accept();
}

void ChartViewBase::keyPressEvent(QKeyEvent* event)
{
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        saveDialog();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void ChartViewBase::setSeriesShow(QAbstractSeries* series, bool shown)
{
    if(shown)
    {
        series->show();
        myAxisX(series)->show();
        myAxisY(series)->show();
    }
    else
    {
        series->hide();
        myAxisX(series)->hide();
        myAxisY(series)->hide();
    }
}

void ChartViewBase::switchToLinAxisY()
{
    const auto logAxisX = qobject_cast<QValueAxis*>(myAxisX(_plottableSeriesLog));
    const auto logAxisY = qobject_cast<QLogValueAxis*>(myAxisY(_plottableSeriesLog));

    const auto rangeX = qMakePair(logAxisX->min(), logAxisX->max());
    const auto rangeY = qMakePair(logAxisY->min(), logAxisY->max());

    setSeriesShow(_plottableSeriesLog, false);
    setSeriesShow(_plottableSeries, true);

    myAxisX(_plottableSeries)->setRange(rangeX.first, rangeX.second);
    myAxisY(_plottableSeries)->setRange(rangeY.first, rangeY.second);
}

void ChartViewBase::switchToLogAxisY()
{
    const auto linAxisX = qobject_cast<QValueAxis*>(myAxisX(_plottableSeries));
    const auto linAxisY = qobject_cast<QValueAxis*>(myAxisY(_plottableSeries));

    const auto rangeX = qMakePair(linAxisX->min(), linAxisX->max());
    const auto rangeY = qMakePair(linAxisY->min(), linAxisY->max());

    setSeriesShow(_plottableSeries, false);
    setSeriesShow(_plottableSeriesLog, true);

    myAxisX(_plottableSeriesLog)->setRange(rangeX.first, rangeX.second);
    myAxisY(_plottableSeriesLog)->setRange(std::max( { rangeY.first,  0.01 } ),
                                           std::max( { rangeY.second, 0.01 } ));
}

bool ChartViewBase::yAxisIsLinear() const
{
    { return _plottableSeries->isVisible(); }
}

void ChartViewBase::mySetAxisX(QAbstractAxis* axisX, QAbstractSeries* series)
{
    _chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
}

void ChartViewBase::mySetAxisY(QAbstractAxis* axisY, QAbstractSeries* series)
{
    _chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}

QAbstractAxis* ChartViewBase::myAxisX(QAbstractSeries* series)
{
    return _chart->axes(Qt::Horizontal, series).first();
}

QAbstractAxis* ChartViewBase::myAxisY(QAbstractSeries* series)
{
    return _chart->axes(Qt::Vertical, series).first();
}


} // namespace gui
} // namespace CTL
