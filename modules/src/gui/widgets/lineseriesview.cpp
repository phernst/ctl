#include "lineseriesview.h"

#include "models/xydataseries.h"

namespace CTL {
namespace gui {

LineSeriesView::LineSeriesView(QWidget* parent)
    : QChartView(parent)
    , _dataSeries(new QLineSeries)
    , _dataSeriesLog(new QLineSeries)
    , _chart(new QChart)
{
    setWindowTitle("Line Series View");

    setChart(_chart);
    _chart->addSeries(_dataSeries);
    _chart->addSeries(_dataSeriesLog);
    _chart->setAxisX(new QValueAxis, _dataSeries);
    _chart->setAxisY(new QValueAxis, _dataSeries);
    _chart->setAxisX(new QValueAxis, _dataSeriesLog);
    _chart->setAxisY(new QLogValueAxis, _dataSeriesLog);

    setSeriesShow(_dataSeriesLog, false);

    _chart->legend()->hide();

    setRubberBand(QChartView::RectangleRubberBand);
}

void LineSeriesView::plot(const XYDataSeries& lineSeries,
                          const QString& labelX, const QString& labelY, bool logAxisY)
{
    auto viewer = new LineSeriesView;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    if(logAxisY)
        viewer->switchToLogAxisY();

    viewer->setData(lineSeries);

    viewer->setLabelX(labelX);
    viewer->setLabelY(labelY);

    viewer->resize(500,400);
    viewer->show();
}

void LineSeriesView::plot(const QList<QPointF>& lineSeries,
                          const QString& labelX, const QString& labelY, bool logAxisY)
{
    plot(XYDataSeries(lineSeries), labelX, labelY, logAxisY);
}

void LineSeriesView::setData(const XYDataSeries& lineSeries)
{
    _dataSeries->clear();
    _dataSeries->append(lineSeries.data());

    if(!yAxisIsLinear())
        updateLogData();

    autoRange();
}

void LineSeriesView::setData(const QList<QPointF>& lineSeries)
{
    setData(XYDataSeries(lineSeries));
}

QImage LineSeriesView::image(const QSize& renderSize)
{
    QSize imgSize = renderSize.isValid() ? renderSize : size();

    QImage ret(imgSize, QImage::Format_ARGB32);
    QPainter painter(&ret);

    render(&painter);

    return ret;
}

void LineSeriesView::autoRange()
{
    auto compareX = [] (const QPointF& a, const QPointF& b) { return a.x()<b.x(); };
    auto compareY = [] (const QPointF& a, const QPointF& b) { return a.y()<b.y(); };

    const auto dataPts = yAxisIsLinear() ? _dataSeries->pointsVector()
                                         : _dataSeriesLog->pointsVector();

    const auto minMaxX = std::minmax_element(dataPts.cbegin(), dataPts.cend(), compareX);
    const auto minMaxY = std::minmax_element(dataPts.cbegin(), dataPts.cend(), compareY);

    const auto xRange = qMakePair(minMaxX.first->x(), minMaxX.second->x());
    const auto yRange = qMakePair(minMaxY.first->y(), minMaxY.second->y());

    if(yAxisIsLinear())
    {
        _chart->axisX(_dataSeries)->setRange(xRange.first, xRange.second);
        _chart->axisY(_dataSeries)->setRange(yRange.first, 1.05 * yRange.second);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(_chart->axisX(_dataSeries))->applyNiceNumbers();
    }
    else
    {
        _chart->axisX(_dataSeriesLog)->setRange(xRange.first, xRange.second);
        _chart->axisY(_dataSeriesLog)->setRange(yRange.first, 1.05 * yRange.second);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(_chart->axisX(_dataSeriesLog))->applyNiceNumbers();
    }
}

void LineSeriesView::setLabelX(const QString& label)
{
    _chart->axisX(_dataSeries)->setTitleText(label);
    _chart->axisX(_dataSeriesLog)->setTitleText(label);
}

void LineSeriesView::setLabelY(const QString& label)
{
    _chart->axisY(_dataSeries)->setTitleText(label);
    _chart->axisY(_dataSeriesLog)->setTitleText(label);
}

void LineSeriesView::setRangeX(double from, double to)
{
    if(yAxisIsLinear())
    {
        _chart->axisX(_dataSeries)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(_chart->axisX(_dataSeries))->applyNiceNumbers();
    }
    else
    {
        _chart->axisX(_dataSeriesLog)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(_chart->axisX(_dataSeriesLog))->applyNiceNumbers();
    }
}

void LineSeriesView::setRangeY(double from, double to)
{
    if(yAxisIsLinear())
        _chart->axisY(_dataSeries)->setRange(from, to);
    else
        _chart->axisY(_dataSeriesLog)->setRange(from, to);
}

void LineSeriesView::setShowPoints(bool enabled)
{
    _dataSeries->setPointsVisible(enabled);
}

void LineSeriesView::setLogAxisY(bool enabled)
{
    enabled ? switchToLogAxisY() : switchToLinAxisY();
}

void LineSeriesView::setUseNiceX(bool enabled) { _useNiceX = enabled; }

void LineSeriesView::toggleLinLogY()
{
    if(yAxisIsLinear())
        switchToLogAxisY();
    else
        switchToLinAxisY();
}

void LineSeriesView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        autoRange();

    event->accept();
}

void LineSeriesView::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
        saveDialog();
}

void LineSeriesView::saveDialog()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save plot", "", "Images (*.png *.jpg *.bmp)");
    if(fn.isEmpty())
        return;

    image().save(fn);
}

bool LineSeriesView::yAxisIsLinear() const { return _dataSeries->isVisible(); }

void LineSeriesView::switchToLinAxisY()
{
    const auto logAxisX = qobject_cast<QValueAxis*>(_chart->axisX(_dataSeriesLog));
    const auto logAxisY = qobject_cast<QLogValueAxis*>(_chart->axisY(_dataSeriesLog));

    const auto rangeX = qMakePair(logAxisX->min(), logAxisX->max());
    const auto rangeY = qMakePair(logAxisY->min(), logAxisY->max());

    setSeriesShow(_dataSeriesLog, false);
    setSeriesShow(_dataSeries, true);

    _chart->axisX(_dataSeries)->setRange(rangeX.first, rangeX.second);
    _chart->axisY(_dataSeries)->setRange(rangeY.first, rangeY.second);
}

void LineSeriesView::switchToLogAxisY()
{
    const auto linAxisX = qobject_cast<QValueAxis*>(_chart->axisX(_dataSeries));
    const auto linAxisY = qobject_cast<QValueAxis*>(_chart->axisY(_dataSeries));

    const auto rangeX = qMakePair(linAxisX->min(), linAxisX->max());
    const auto rangeY = qMakePair(linAxisY->min(), linAxisY->max());

    updateLogData();

    setSeriesShow(_dataSeries, false);
    setSeriesShow(_dataSeriesLog, true);

    _chart->axisX(_dataSeriesLog)->setRange(rangeX.first, rangeX.second);
    _chart->axisY(_dataSeriesLog)->setRange(std::max( { rangeY.first,  0.01 } ),
                                         std::max( { rangeY.second, 0.01 } ));
}

void LineSeriesView::updateLogData()
{
    _dataSeriesLog->clear();
    for(const auto& pt : _dataSeries->pointsVector())
        if(pt.y() > 0.0)
            _dataSeriesLog->append(pt);
}

void LineSeriesView::setSeriesShow(QAbstractSeries* series, bool shown)
{
    if(shown)
    {
        series->show();
        _chart->axisX(series)->show();
        _chart->axisY(series)->show();
    }
    else
    {
        series->hide();
        _chart->axisX(series)->hide();
        _chart->axisY(series)->hide();
    }
}

} // namespace gui
} // namespace CTL
