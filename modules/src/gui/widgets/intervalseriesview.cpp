#include "intervalseriesview.h"

#include "models/intervaldataseries.h"

#include <limits>

namespace CTL {
namespace gui {

IntervalSeriesView::IntervalSeriesView(QWidget* parent)
    : QChartView(parent)
    , _areaSeries(new QAreaSeries)
    , _areaSeriesLog(new QAreaSeries)
    , _upper(new QLineSeries)
    , _upperLog(new QLineSeries)
    , _chart(new QChart)
{
    _areaSeries->setUpperSeries(_upper);
    _areaSeries->setColor(Qt::lightGray);
    _areaSeries->setBorderColor(Qt::darkGray);

    _areaSeriesLog->setUpperSeries(_upperLog);
    _areaSeriesLog->setColor(Qt::lightGray);
    _areaSeriesLog->setBorderColor(Qt::darkGray);

    _chart->addSeries(_areaSeries);
    _chart->addSeries(_areaSeriesLog);
    _chart->legend()->hide();

    _chart->setAxisX(new QValueAxis, _areaSeries);
    _chart->setAxisY(new QValueAxis, _areaSeries);
    _chart->setAxisX(new QValueAxis, _areaSeriesLog);
    _chart->setAxisY(new QLogValueAxis, _areaSeriesLog);

    setSeriesShow(_areaSeriesLog, false);

    setChart(_chart);
    setRubberBand(QChartView::RectangleRubberBand);
    setWindowTitle("Bar Series View");
}

void IntervalSeriesView::plot(const IntervalDataSeries& intervalSeries,
                         const QString& labelX, const QString& labelY, bool logAxisY)
{
    auto viewer = new IntervalSeriesView;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    if(logAxisY)
        viewer->switchToLogAxisY();

    viewer->setData(intervalSeries);

    viewer->_chart->axisX()->setTitleText(labelX);
    viewer->_chart->axisY()->setTitleText(labelY);

    viewer->resize(500,400);
    viewer->show();
}

void IntervalSeriesView::setData(const IntervalDataSeries& intervalSeries)
{
    _upper->clear();
    _upperLog->clear();

    static const auto eps = 0.0001;
    const auto binWidth = intervalSeries.binWidth();
    const auto logMinVal = suitableLogMinVal(intervalSeries);

    for(const auto& pt : intervalSeries.data())
    {
        _upper->append(pt.x() - (0.5 - eps) * binWidth, pt.y());
        _upper->append(pt.x() + 0.5 * binWidth, pt.y());
        _upper->append(pt.x() + (0.5 + eps) * binWidth, 0.0);

        const auto clampedLogVal = std::max( { pt.y(), logMinVal } );
        _upperLog->append(pt.x() - (0.5 - eps) * binWidth, clampedLogVal);
        _upperLog->append(pt.x() + 0.5 * binWidth, clampedLogVal);
        _upperLog->append(pt.x() + (0.5 + eps) * binWidth, logMinVal);
    }

    autoRange();
}

QImage IntervalSeriesView::image(const QSize& renderSize)
{
    QSize imgSize = renderSize.isValid() ? renderSize : size();

    QImage ret(imgSize, QImage::Format_RGB32);
    QPainter painter(&ret);

    auto bgBrush = backgroundBrush();

    setBackgroundBrush(QBrush(Qt::white));
    render(&painter);
    setBackgroundBrush(bgBrush);

    return ret;
}

void IntervalSeriesView::autoRange()
{
    auto compareX = [] (const QPointF& a, const QPointF& b) { return a.x()<b.x(); };
    auto compareY = [] (const QPointF& a, const QPointF& b) { return a.y()<b.y(); };

    const auto dataPts = yAxisIsLinear() ? _upper->pointsVector()
                                         : _upperLog->pointsVector();

    const auto minMaxX = std::minmax_element(dataPts.cbegin(), dataPts.cend(), compareX);
    const auto minMaxY = std::minmax_element(dataPts.cbegin(), dataPts.cend(), compareY);

    const auto xRange = qMakePair(minMaxX.first->x(), minMaxX.second->x());
    const auto yRange = qMakePair(minMaxY.first->y(), minMaxY.second->y());

    setRangeX(xRange.first, xRange.second);
    setRangeY(yRange.first, 1.05 * yRange.second);
}

void IntervalSeriesView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
        autoRange();

    event->accept();
}

void IntervalSeriesView::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        saveDialog();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

bool IntervalSeriesView::save(const QString& fileName)
{
    return image().save(fileName);
}

void IntervalSeriesView::saveDialog()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save plot", "", "Images (*.png *.jpg *.bmp)");
    if(fn.isEmpty())
        return;

    save(fn);
}

void IntervalSeriesView::setLabelX(const QString& label)
{
    _chart->axisX(_areaSeries)->setTitleText(label);
    _chart->axisX(_areaSeriesLog)->setTitleText(label);
}

void IntervalSeriesView::setLabelY(const QString& label)
{
    _chart->axisY(_areaSeries)->setTitleText(label);
    _chart->axisY(_areaSeriesLog)->setTitleText(label);
}

double IntervalSeriesView::suitableLogMinVal(const IntervalDataSeries& intervalSeries)
{
    static const auto bottomScale = 0.01;

    std::vector<double> tmp;
    tmp.reserve(intervalSeries.size());

    for(const auto pt : intervalSeries.data())
        if(pt.y() > 0.0)
            tmp.push_back(pt.y());

    const auto minVal = *std::min_element(tmp.cbegin(), tmp.cend());

    return std::max(bottomScale * minVal, std::numeric_limits<double>::min());
}

void IntervalSeriesView::setLogAxisY(bool enabled)
{
    enabled ? switchToLogAxisY() : switchToLinAxisY();
}

void IntervalSeriesView::setRangeX(double from, double to)
{
    if(yAxisIsLinear())
    {
        _chart->axisX(_areaSeries)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(_chart->axisX(_areaSeries))->applyNiceNumbers();
    }
    else
    {
        _chart->axisX(_areaSeriesLog)->setRange(from, to);
        if(_useNiceX)
            qobject_cast<QValueAxis*>(_chart->axisX(_areaSeriesLog))->applyNiceNumbers();
    }
}

void IntervalSeriesView::setRangeY(double from, double to)
{
    if(yAxisIsLinear())
        _chart->axisY(_areaSeries)->setRange(from, to);
    else
        _chart->axisY(_areaSeriesLog)->setRange(from, to);
}

void IntervalSeriesView::setUseNiceX(bool enabled) { _useNiceX = enabled; }

void IntervalSeriesView::toggleLinLogY()
{
    if(yAxisIsLinear())
        switchToLogAxisY();
    else
        switchToLinAxisY();
}

void IntervalSeriesView::setSeriesShow(QAbstractSeries* series, bool shown)
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

bool IntervalSeriesView::yAxisIsLinear() const { return _areaSeries->isVisible(); }

void IntervalSeriesView::switchToLinAxisY()
{
    const auto logAxisX = qobject_cast<QValueAxis*>(_chart->axisX(_areaSeriesLog));
    const auto logAxisY = qobject_cast<QLogValueAxis*>(_chart->axisY(_areaSeriesLog));

    const auto rangeX = qMakePair(logAxisX->min(), logAxisX->max());
    const auto rangeY = qMakePair(logAxisY->min(), logAxisY->max());

    setSeriesShow(_areaSeriesLog, false);
    setSeriesShow(_areaSeries, true);

    _chart->axisX(_areaSeries)->setRange(rangeX.first, rangeX.second);
    _chart->axisY(_areaSeries)->setRange(rangeY.first, rangeY.second);
}

void IntervalSeriesView::switchToLogAxisY()
{
    const auto linAxisX = qobject_cast<QValueAxis*>(_chart->axisX(_areaSeries));
    const auto linAxisY = qobject_cast<QValueAxis*>(_chart->axisY(_areaSeries));

    const auto rangeX = qMakePair(linAxisX->min(), linAxisX->max());
    const auto rangeY = qMakePair(linAxisY->min(), linAxisY->max());

    setSeriesShow(_areaSeries, false);
    setSeriesShow(_areaSeriesLog, true);

    _chart->axisX(_areaSeriesLog)->setRange(rangeX.first, rangeX.second);
    _chart->axisY(_areaSeriesLog)->setRange(std::max( { rangeY.first,  0.01 } ),
                                         std::max( { rangeY.second, 0.01 } ));
}

} // namespace gui
} // namespace CTL
