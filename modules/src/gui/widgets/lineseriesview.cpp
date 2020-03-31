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
    mySetAxisX(new QValueAxis, _dataSeries);
    mySetAxisY(new QValueAxis, _dataSeries);
    mySetAxisX(new QValueAxis, _dataSeriesLog);
    mySetAxisY(new QLogValueAxis, _dataSeriesLog);

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
        myAxisX(_dataSeries)->setRange(xRange.first, xRange.second);
        myAxisY(_dataSeries)->setRange(yRange.first, 1.05 * yRange.second);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(myAxisX(_dataSeries))->applyNiceNumbers();
    }
    else
    {
        myAxisX(_dataSeriesLog)->setRange(xRange.first, xRange.second);
        myAxisY(_dataSeriesLog)->setRange(yRange.first, 1.05 * yRange.second);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(myAxisX(_dataSeriesLog))->applyNiceNumbers();
    }
}

void LineSeriesView::setLabelX(const QString& label)
{
    myAxisX(_dataSeries)->setTitleText(label);
    myAxisX(_dataSeriesLog)->setTitleText(label);
}

void LineSeriesView::setLabelY(const QString& label)
{
    myAxisY(_dataSeries)->setTitleText(label);
    myAxisY(_dataSeriesLog)->setTitleText(label);
}

void LineSeriesView::setRangeX(double from, double to)
{
    if(yAxisIsLinear())
    {
        myAxisX(_dataSeries)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(myAxisX(_dataSeries))->applyNiceNumbers();
    }
    else
    {
        myAxisX(_dataSeriesLog)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(myAxisX(_dataSeriesLog))->applyNiceNumbers();
    }
}

void LineSeriesView::setRangeY(double from, double to)
{
    if(yAxisIsLinear())
        myAxisY(_dataSeries)->setRange(from, to);
    else
        myAxisY(_dataSeriesLog)->setRange(from, to);
}

void LineSeriesView::setShowPoints(bool enabled)
{
    _dataSeries->setPointsVisible(enabled);
    _dataSeriesLog->setPointsVisible(enabled);
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
    {
        saveDialog();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

bool LineSeriesView::save(const QString& fileName)
{
    return image().save(fileName);
}

void LineSeriesView::saveDialog()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save plot", "", "Images (*.png *.jpg *.bmp)");
    if(fn.isEmpty())
        return;

    save(fn);
}

bool LineSeriesView::yAxisIsLinear() const { return _dataSeries->isVisible(); }

void LineSeriesView::mySetAxisX(QAbstractAxis* axisX, QAbstractSeries* series)
{
    _chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
}

void LineSeriesView::mySetAxisY(QAbstractAxis* axisY, QAbstractSeries* series)
{
    _chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}

QAbstractAxis* LineSeriesView::myAxisX(QAbstractSeries* series)
{
    return _chart->axes(Qt::Horizontal, series).first();
}

QAbstractAxis* LineSeriesView::myAxisY(QAbstractSeries* series)
{
    return _chart->axes(Qt::Vertical, series).first();
}

void LineSeriesView::switchToLinAxisY()
{
    const auto logAxisX = qobject_cast<QValueAxis*>(myAxisX(_dataSeriesLog));
    const auto logAxisY = qobject_cast<QLogValueAxis*>(myAxisY(_dataSeriesLog));

    const auto rangeX = qMakePair(logAxisX->min(), logAxisX->max());
    const auto rangeY = qMakePair(logAxisY->min(), logAxisY->max());

    setSeriesShow(_dataSeriesLog, false);
    setSeriesShow(_dataSeries, true);

    myAxisX(_dataSeries)->setRange(rangeX.first, rangeX.second);
    myAxisY(_dataSeries)->setRange(rangeY.first, rangeY.second);
}

void LineSeriesView::switchToLogAxisY()
{
    const auto linAxisX = qobject_cast<QValueAxis*>(myAxisX(_dataSeries));
    const auto linAxisY = qobject_cast<QValueAxis*>(myAxisY(_dataSeries));

    const auto rangeX = qMakePair(linAxisX->min(), linAxisX->max());
    const auto rangeY = qMakePair(linAxisY->min(), linAxisY->max());

    updateLogData();

    setSeriesShow(_dataSeries, false);
    setSeriesShow(_dataSeriesLog, true);

    myAxisX(_dataSeriesLog)->setRange(rangeX.first, rangeX.second);
    myAxisY(_dataSeriesLog)->setRange(std::max( { rangeY.first,  0.01 } ),
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

} // namespace gui
} // namespace CTL
